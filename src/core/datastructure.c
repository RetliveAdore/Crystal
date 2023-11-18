#include <Crystal.h>
#include <malloc.h>
#include <string.h>
#include <crerrors.h>

#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#endif

#define DYN 0x00
#define TRE 0x01
#define LIN 0x02
#define LOO 0x03

//容量限制是512MB
#define DYN_MAX (sizeof(CRUINT8) << 29)

#define GET_MOD(num, mod) { if ((num) < 0) (num) = -(-(num) % (mod)); else if ((num > 0)) (num) = ((num) % (mod)); }

#include <stdio.h>
FILE* fp = NULL;

//共有的数据头
typedef struct
{
#ifdef CR_WINDOWS
	CRITICAL_SECTION cs;  //确保多线程安全
#elif defined CR_LINUX
	pthread_mutex_t cs;  //确保多线程安全
#endif
	CRUINT8 type;  //用于表示据结构类型
	CRUINT32 total;  //用于表示现在有多少个元素
} CRSTRUCTUREPUB;

#ifdef CR_LINUX  //直接使出那一招，简单方便又快捷
void InitializeCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_init(mt, NULL);
}
void DeleteCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_destroy(mt);
}
void EnterCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_lock(mt);
}
void LeaveCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_unlock(mt);
}
#endif

//

typedef struct
{
	CRSTRUCTUREPUB pub;
	CRUINT8* arr;
	CRUINT32 capacity;
}CRDYN, *PCRDYN;

//

typedef struct treenode
{
	CRINT64 key;
	CRLVOID data;
	CRBOOL red;
	struct treenode* parent;
	struct treenode* left;
	struct treenode* right;
}TREENODE, *PTREENODE;

typedef struct
{
	CRSTRUCTUREPUB pub;
	PTREENODE root;
}CRTREE, *PCRTREE;

//

typedef struct linearnode
{
	CRLVOID data;
	struct linearnode* prior;
	struct linearnode* after;
}LINEARNODE, *PLINEARNODE;

typedef struct
{
	CRSTRUCTUREPUB pub;
	PLINEARNODE hook;
}CRLINEAR, *PCRLINEAR;

//

typedef struct
{
	CRSTRUCTURE pub;
	CRUINT8* arr;
}CRLOOP, *PCRLOOP;

//

CRAPI CRSTRUCTURE CRDynamic()
{
	PCRDYN pInner = malloc(sizeof(CRDYN));
	if (!pInner)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return NULL;
	}
	pInner->arr = malloc(sizeof(CRUINT8));
	if (!pInner->arr)
	{
		free(pInner);
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return NULL;
	}
	pInner->pub.type = DYN;
	pInner->pub.total = 0;
	InitializeCriticalSection(&(pInner->pub.cs));
	pInner->arr[0] = 0;
	pInner->capacity = 1;
	return pInner;
}

CRAPI CRSTRUCTURE CRTree()
{
	PCRTREE pInner = malloc(sizeof(CRTREE));
	if (!pInner)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return NULL;
	}
	pInner->pub.type = TRE;
	pInner->pub.total = 0;
	InitializeCriticalSection(&(pInner->pub.cs));
	pInner->root = NULL;
	return pInner;
}

CRAPI CRSTRUCTURE CRLinear()
{
	PCRLINEAR pInner = malloc(sizeof(CRLINEAR));
	if (!pInner)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return NULL;
	}
	pInner->pub.type = LIN;
	pInner->pub.total = 0;
	InitializeCriticalSection(&(pInner->pub.cs));
	pInner->hook = NULL;
	return pInner;
}
//现在还用不着，就先放着不管了
CRAPI CRSTRUCTURE CRLoop()
{
	return NULL;
}

//
//
//动态数组的实现
//

CRUINT8* _sizeup_cr_(CRUINT8* arr, CRUINT32 size, CRUINT32 capacity)
{
	CRUINT8* tmp = calloc(capacity, sizeof(CRUINT8));
	if (!tmp) return NULL;
	memcpy(tmp, arr, size);
	free(arr);
	return tmp;
}

CRUINT8* _sizedown_cr_(CRUINT8* arr, CRUINT32 capacity)
{
	//如果是缩小内存块，内容保留，直接将后面的块拿掉，不会出现内存不足的问题
	return realloc(arr, capacity);
}

//

/*
* private API
这几个函数不是给用户用的，是给内部人员用的。
所以说没有写到头文件里面
因为内存管理是：谁申请谁释放，在扩展库里面释放内存会出问题
*/

CRAPI CRCODE CRDynSizeUp(CRSTRUCTURE dyn, CRUINT32 size, CRUINT32 capacity)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	CRUINT8* tmp = calloc(capacity, sizeof(CRUINT8));
	if (!tmp) return CRERR_OUTOFMEM;
	memcpy(tmp, pInner->arr, size);
	free(pInner->arr);
	pInner->arr = tmp;
	return 0;
}
CRAPI CRCODE CRDynSizeDown(CRSTRUCTURE dyn, CRUINT32 capacity)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	if (!realloc(pInner->arr, capacity))
		return CRERR_OUTOFMEM;
	return 0;
}
CRAPI CRUINT8* CRDynArr(CRSTRUCTURE dyn, CRUINT32* size)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
	{
		if (size) *size = 0;
		return NULL;
	}
	if (size) *size = pInner->pub.total;
	return pInner->arr;
}

CRAPI CRCODE CRDynPush(CRSTRUCTURE dyn, CRUINT8 data)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (pInner->pub.total < pInner->capacity)
	{
		pInner->arr[pInner->pub.total++] = data;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	}
	//需要扩容的情况，不得超过容量限制
	if (pInner->capacity >= DYN_MAX)
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_STRUCTURE_FULL;
	}
	pInner->capacity <<= 1;
	CRUINT8* tmp = _sizeup_cr_(pInner->arr, pInner->pub.total, pInner->capacity);
	if (!tmp)
	{
		CRThrowError(CRERR_STRUCTURE_RESIZE, CRDES_STRUCTURE_RESIZE);
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_STRUCTURE_RESIZE;
	}
	tmp[pInner->pub.total++] = data;
	pInner->arr = tmp;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRDynSet(CRSTRUCTURE dyn, CRUINT8 data, CRUINT32 sub)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (sub < pInner->pub.total)
		pInner->arr[sub] = data;
	else if (sub < pInner->capacity)
	{
		pInner->arr[sub] = data;
		pInner->pub.total = sub + 1;
	}
	else if (sub + 1 < DYN_MAX)
	{
		while (pInner->capacity < sub + 1) pInner->capacity <<= 1;
		CRUINT8* tmp = _sizeup_cr_(pInner->arr, pInner->pub.total, pInner->capacity);
		if (!tmp)
		{
			LeaveCriticalSection(&(pInner->pub.cs));
			return CRERR_OUTOFMEM;
		}
		tmp[sub++] = data;
		pInner->arr = tmp;
		pInner->pub.total = sub;
	}
	else
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_STRUCTURE_FULL;
	}
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRDynPop(CRSTRUCTURE dyn, CRUINT8* data)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (pInner->pub.total == 0)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	pInner->pub.total--;
	if (data) *data = pInner->arr[pInner->pub.total];
	if (pInner->pub.total < pInner->capacity >> 1 && pInner->capacity > 32)//可以释放一些空间
	{
		pInner->capacity >>= 1;
		pInner->arr = _sizedown_cr_(pInner->arr, pInner->capacity);
	}
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRDynSeek(CRSTRUCTURE dyn, CRUINT8* data, CRUINT32 sub)
{
	PCRDYN pInner = dyn;
	if (pInner && pInner->pub.type == DYN)
	{
		if (!data)
			return CRERR_INVALID;
		EnterCriticalSection(&(pInner->pub.cs));
		if (sub < pInner->pub.total)
			*data = pInner->arr[sub];
		else
			*data = 0;
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	}
	return CRERR_NOTFOUND;
}

CRAPI CRUINT8* CRDynCopy(CRSTRUCTURE dyn, CRUINT32* size)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return NULL;
	}
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->pub.total)
	{
		CRThrowError(CRERR_NOTFOUND, NULL);
		LeaveCriticalSection(&(pInner->pub.cs));
		return NULL;
	}
	CRUINT8* out = malloc(pInner->pub.total * sizeof(CRUINT8));
	if (!out)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		LeaveCriticalSection(&(pInner->pub.cs));
		return NULL;
	}
	memcpy(out, pInner->arr, pInner->pub.total * sizeof(CRUINT8));
	if (size) *size = pInner->pub.total;
	LeaveCriticalSection(&(pInner->pub.cs));
	return out;
}

CRAPI void CRDynFreeCopy(CRUINT8* data)
{
	free(data);
}

CRAPI CRCODE CRDynSetup(CRSTRUCTURE dyn, CRUINT8* buffer, CRUINT32 size)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	if (size > DYN_MAX)
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		return CRERR_STRUCTURE_FULL;
	}
	EnterCriticalSection(&(pInner->pub.cs));
	if (!buffer)
	{
		CRUINT8* tmp = calloc(pInner->capacity, sizeof(CRUINT8));
		if (!tmp)
		{
			LeaveCriticalSection(&(pInner->pub.cs));
			return CRERR_OUTOFMEM;
		}
		free(pInner->arr);
		pInner->arr = tmp;
		pInner->pub.total = 0;
		pInner->capacity = 1;
		goto Done;
	}
	pInner->capacity = 1;
	while (pInner->capacity < size) pInner->capacity <<= 1;
	CRUINT8* tmp = calloc(pInner->capacity, sizeof(CRUINT8));
	if (!tmp)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_OUTOFMEM;
	}
	memcpy(tmp, buffer, size);
	free(pInner->arr);
	pInner->arr = tmp;
	pInner->pub.total = size;
Done:
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

//
//
//键值树的实现
//

PTREENODE _create_treenode_cr_(CRINT64 key, CRLVOID data, PTREENODE parent)
{
	PTREENODE pNode = malloc(sizeof(TREENODE));
	if (pNode)
	{
		pNode->parent = parent;
		pNode->red = CRTRUE;
		pNode->key = key;
		pNode->data = data;
		pNode->left = NULL;
		pNode->right = NULL;
	}
	return pNode;
}
//当且仅当node是左节点时返回CRTRUE，其余所有情况返回CRFALSE
static inline CRBOOL _left_child_(PTREENODE node)
{
	if (node && node->parent && node == node->parent->left)
		return CRTRUE;
	return CRFALSE;
}
//当且仅当node存在且颜色为红色时返回CRTRUE
static inline CRBOOL _color_(PTREENODE node)
{
	return node == NULL ? CRFALSE : node->red;
}
//兄弟结点
static inline PTREENODE _sibling_(PTREENODE node)
{
	if (_left_child_(node))
		return node->parent->right;
	else if (node->parent)
		return node->parent->left;
	return NULL;
}
//左旋操作，顺便集成了根结点的判定
void _left_rotate_(PTREENODE node, PCRTREE tree)
{
	PTREENODE top = node, right = node->right, child = right->left;
	if (!node->parent)
		tree->root = right;
	else if (_left_child_(node))
		node->parent->left = right;
	else node->parent->right = right;
	right->parent = top->parent;
	right->left = top;
	top->parent = right;
	top->right = child;
	if (child)
		child->parent = top;
}
//右旋操作，顺便集成了根结点的判定
void _right_rotate_(PTREENODE node, PCRTREE tree)
{
	PTREENODE top = node, left = node->left, child = left->right;
	if (!node->parent)
		tree->root = left;
	else if (_left_child_(node))
		node->parent->left = left;
	else node->parent->right = left;
	left->parent = top->parent;
	left->right = top;
	top->parent = left;
	top->left = child;
	if (child)
		child->parent = top;
}
//获取前驱结点
PTREENODE _prev_node_(PTREENODE node)
{
	/*
	* 此处不需要考虑向上寻找结点，因为在删除情况下不会用到这种情况
	* 在删除时，只要需要寻找前驱节点，就说明这个节点下方必然有两个节点
	*/
	node = node->left;
	while (node->right) node = node->right;
	return node;
}
//寻找对应结点，假如能够找到，返回此结点
PTREENODE _look_up_(PTREENODE root, CRINT64 key, PTREENODE* parent)
{
	*parent = root;
Find:
	if (root->key == key)
		return root;
	else if (root->key < key && root->right)
	{
		root = root->right;
		*parent = root;
		goto Find;
	}
	else if (root->key > key && root->left)
	{
		root = root->left;
		*parent = root;
		goto Find;
	}
	return NULL;
}

//吐槽：插入操作比删除操作简单多了
CRAPI CRCODE CRTreePut(CRSTRUCTURE tree, CRLVOID data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		pInner->root = _create_treenode_cr_(key, data, NULL);
		if (!pInner->root)
		{
			LeaveCriticalSection(&(pInner->pub.cs));
			return CRERR_OUTOFMEM;
		}
		pInner->root->red = CRFALSE;
		goto Done;
	}
	PTREENODE node = NULL, parent = NULL, uncle = NULL, grandpa = NULL;
	if (node = _look_up_(pInner->root, key, &parent))
	{
		node->data = data;
		goto Done;
	}
	node = _create_treenode_cr_(key, data, parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_OUTOFMEM;
	}
	if (key < parent->key)
		parent->left = node;
	else parent->right = node;
Fix:
	if (node == pInner->root)
	{
		node->red = CRFALSE;
		goto Done;
	}
	if (!_color_(node->parent))
	{
		goto Done;
	}
	parent = node->parent;
	uncle = _sibling_(parent);
	grandpa  = parent->parent;
	if (_color_(uncle))
	{
		parent->red = CRFALSE;
		uncle->red = CRFALSE;
		grandpa->red = CRTRUE;
		node = grandpa;
		goto Fix;
	}
	else
	{
		if (_left_child_(parent))
		{
			if (!_left_child_(node))
			{
				_left_rotate_(parent, pInner);
				parent = node;
				node = node->left;
			}
			parent->red = CRFALSE;
			grandpa->red = CRTRUE;
			_right_rotate_(grandpa, pInner);
		}
		else
		{
			if (_left_child_(node))
			{
				_right_rotate_(parent, pInner);
				parent = node;
				node = node->right;
			}
			parent->red = CRFALSE;
			grandpa->red = CRTRUE;
			_left_rotate_(grandpa, pInner);
		}
	}
Done:
	pInner->pub.total++;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

//
//噩梦——删除红黑树节点

//declaration
void _case_1_red_sibling_(PCRTREE tree, PTREENODE node);
void _case_2_far_red_nephew_(PCRTREE tree, PTREENODE node);
void _case_3_near_red_nephew(PCRTREE tree, PTREENODE node);
void _case_4_red_parent_(PCRTREE tree, PTREENODE node);
void _case_5_all_black(PCRTREE tree, PTREENODE node);

/*假如兄弟结点是红色，就转化成父节点是红色的情况，以统一处理
     p              S   |      p             S
    / \            / \  |     / \           / \
   D  red   ==>  red ...|   red  D   ==>  ... red
  ... /  \       / \    |   /  \ ...          / \
 .. ...  ...    D  ...  | ...  ... ..       ...  D
*/
void _case_1_red_sibling_(PCRTREE tree, PTREENODE node)
{
	node->parent->red = CRTRUE;
	if (_left_child_(node))
	{
		node->parent->right->red = CRFALSE;
		_left_rotate_(node->parent, tree);
	}
	else
	{
		node->parent->left->red = CRFALSE;
		_right_rotate_(node->parent, tree);
	}
}
/*兄弟节点是黑色且挂了一个远端红色侄子结点
     p              p
    / \            / \
   D   S          S   D
  ... / \        / \  ...
 .. ... red    red ...  ..
*/
void _case_2_far_red_nephew_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	PTREENODE farNephew;
	sibling->red = node->parent->red;
	node->parent->red = CRFALSE;
	if (_left_child_(node))
	{
		farNephew = sibling->right;
		_left_rotate_(node->parent, tree);
	}
	else
	{
		farNephew = sibling->left;
		_right_rotate_(node->parent, tree);
	}
	farNephew->red = CRFALSE;
}
/*兄弟节点设计黑色且只挂了一个近端红色侄子节点
	 p              p
    / \            / \
   D   S          S   D
  ... / \        / \  ...
 .. red  ...   ...  red ..
转化成为情况2来处理
*/
void _case_3_near_red_nephew(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	PTREENODE nearNephew;
	if (_left_child_(node))
	{
		nearNephew = sibling->left;
		_right_rotate_(sibling, tree);
	}
	else
	{
		nearNephew = sibling->right;
		_left_rotate_(sibling, tree);
	}
	sibling->red = CRTRUE;
	nearNephew->red = CRFALSE;
	//转化之后就可以使用情况2来处理了
	_case_2_far_red_nephew_(tree, node);
}
/*父结点为红色而且有兄弟结点
* 将父节点变为黑色，兄弟节点变红就能平衡
    red          red
    / \          / \
   D   S        S   D
  / \ / \      / \ / \
 .........    .........
*/
void _case_4_red_parent_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	node->parent->red = CRFALSE;
	sibling->red = CRTRUE;
}
/*唯一一种需要用到迭代的情况，全黑
   Black        Black
    / \          / \
   D Black    Black D
  / \ / \      / \ / \
 .........    .........
*/
void _case_5_all_black(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	sibling->red = CRTRUE;
}

void _fix_single_black_node_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling;
Fix:
	if (node == tree->root)
	{
		node->red = CRFALSE;
		return;
	}
	//这种情况下兄弟节点必然存在，否则就不平衡了
	sibling = _sibling_(node);
	if (_color_(sibling))
	{
		_case_1_red_sibling_(tree, node);
		goto Fix;
	}
	else if (_left_child_(node))
	{
		if (_color_(sibling->right))
			_case_2_far_red_nephew_(tree, node);
		else if (_color_(sibling->left))
			_case_3_near_red_nephew(tree, node);
		else if (_color_(node->parent))
			_case_4_red_parent_(tree, node);
		else
		{
			_case_5_all_black(tree, node);
			node = node->parent;
			goto Fix;
		}
	}
	else
	{
		if (_color_(sibling->left))
			_case_2_far_red_nephew_(tree, node);
		else if (_color_(sibling->right))
			_case_3_near_red_nephew(tree, node);
		else if (_color_(node->parent))
			_case_4_red_parent_(tree, node);
		else
		{
			_case_5_all_black(tree, node);
			node = node->parent;
			goto Fix;
		}
	}
}

void _fix_parent_(PTREENODE node)
{
	if (_left_child_(node))
		node->parent->left = NULL;
	else if (node->parent)
		node->parent->right = NULL;
}

CRAPI CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	PTREENODE parent;
	PTREENODE node = _look_up_(pInner->root, key, &parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	if (data) *data = node->data;
Fix:
	if (node->left && node->right)
	{
		PTREENODE prev = _prev_node_(node);
		node->data = prev->data;
		node->key = prev->key;
		node = prev;
	}  //假如红黑树的结构是正确的，转换一次就够了
	if (node->red)
		goto Done;
	else if (node->left)
	{
		node->data = node->left->data;
		node->key = node->left->key;
		node = node->left;
	}
	else if (node->right)
	{
		node->data = node->right->data;
		node->key = node->right->key;
		node = node->right;
	}
	else
		_fix_single_black_node_(pInner, node);
Done:
	_fix_parent_(node);
	if (node == pInner->root)
		pInner->root = NULL;
	free(node);
	pInner->pub.total--;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRTreeSeek(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	PTREENODE parent;
	PTREENODE node = _look_up_(pInner->root, key, &parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	if (data) *data = node->data;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

//
// 
//线性表实现
//

typedef void (*TrashBinFunc)(void);

CRAPI CRCODE CRLinPut(CRSTRUCTURE lin, CRLVOID data, CRINT64 seek)
{
	PCRLINEAR pInner = lin;
	if (!pInner || pInner->pub.type != LIN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	PLINEARNODE ins = malloc(sizeof(LINEARNODE));
	if (!ins)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_OUTOFMEM;
	}
	ins->data = data;
	GET_MOD(seek, pInner->pub.total);
	if (!pInner->hook)
	{
		ins->prior = ins;
		ins->after = ins;
		pInner->hook = ins;
	}
	else if (seek < 0)
	{
		ins->prior = pInner->hook->prior;
		while (seek < 0) { seek++; ins->prior = ins->prior->prior; }
		ins->after = ins->prior->after;
		ins->prior->after = ins;
		ins->after->prior = ins;
	}
	else
	{
		ins->after = pInner->hook->after;
		while (seek > 0) { seek--; ins->after = ins->after->after; }
		ins->prior = ins->after->prior;
		ins->after->prior = ins;
		ins->prior->after = ins;
	}
	pInner->pub.total++;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRLinSeek(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek)
{
	PCRLINEAR pInner = lin;
	if (!pInner || pInner->pub.type != LIN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->hook)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	PLINEARNODE node = pInner->hook;
	GET_MOD(seek, pInner->pub.total);
	if (seek < 0)
		while (seek < 0) { seek++; node = node->prior; }
	else
		while (seek > 0) { seek--; node = node->after; }
	if (data) *data = node->data;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRLinGet(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek)
{
	PCRLINEAR pInner = lin;
	if (!pInner || pInner->pub.type != LIN)
		return CRERR_INVALID;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->hook)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return CRERR_NOTFOUND;
	}
	PLINEARNODE node = pInner->hook;
	GET_MOD(seek, pInner->pub.total);
	if (seek < 0)
		while (seek < 0) { seek++; node = node->prior; }
	else
		while (seek > 0) { seek--; node = node->after; }
	if (data) *data = node->data;
	if (pInner->pub.total == 1)
	{
		free(pInner->hook);
		pInner->hook = NULL;
	}
	else
	{
		if (node == pInner->hook)
			pInner->hook = node->after;
		node->prior->after = node->after;
		node->after->prior = node->prior;
		free(node);
	}
	pInner->pub.total--;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

//什么也不做，仅占位
void _struc_do_nothing_(CRLVOID data) { return; }
void _clear_nothing_(CRSTRUCTURE s, DSCallback cal) { return; }

void _clear_dyn_(CRSTRUCTURE s, DSCallback cal)
{
	PCRDYN dyn = s;
	for (int i = 0; i < dyn->pub.total; i++) cal((CRLVOID)(CRUINT64)(dyn->arr[i]));
	free(dyn->arr);
}

void _clear_tree_node_(PTREENODE node, DSCallback cal)
{
	if (node->left)
		_clear_tree_node_(node->left, cal);
	if (node->right)
		_clear_tree_node_(node->right, cal);
	cal(node->data);
	free(node);
}
void _clear_tree_(CRSTRUCTURE s, DSCallback cal)
{
	PCRTREE tree = s;
	if (tree->root)
		_clear_tree_node_(tree->root, cal);
}

void _clear_linear_(CRSTRUCTURE s, DSCallback cal)
{
	PCRLINEAR lin = s;
	if (!lin->hook)
		return;
	PLINEARNODE node = lin->hook, p = node;
	node->prior->after = NULL;
	while (node->after)
	{
		p = node;
		node = node->after;
		cal(node->data);
		free(p);
	}
	cal(node->data);
	free(node);
}

//使用一下数据思想
typedef void (*_Struct_Clear_Func_)(CRSTRUCTURE s, DSCallback cal);
const _Struct_Clear_Func_ clearFuncs[] =
{
	_clear_dyn_,
	_clear_tree_,
	_clear_linear_,
	_clear_nothing_
};

CRAPI CRCODE CRFreeStructure(CRSTRUCTURE s, DSCallback cal)
{
	CRSTRUCTUREPUB* pub = s;
	if (!pub)
		return CRERR_INVALID;
	//如此操作只是为了确保最后一个操作完成，假如后面还有操作，造成的错误概不负责
	EnterCriticalSection(&(pub->cs));
	LeaveCriticalSection(&(pub->cs));
	DeleteCriticalSection(&(pub->cs));
	if (!cal)
		cal = _struc_do_nothing_;
	if (pub->type < 0 || pub->type > 3)
		return CRERR_INVALID;
	clearFuncs[pub->type](s, cal);
	free(s);
	return 0;
}

CRAPI CRUINT32 CRStructureSize(CRSTRUCTURE s)
{
	if (s)
		return ((CRSTRUCTUREPUB*)s)->total;
	return 0;
}