#include <Crystal.h>
#include <malloc.h>
#include <string.h>
#include <crerrors.h>

#define DYN 0x00
#define TRE 0x01
#define LIN 0x02
#define LOO 0x03

//容量限制是512MB
#define DYN_MAX (sizeof(CRUINT8) << 29)

#define IF_FAILED_QUIT(p) if(!(p)) {CRThrowError(CRERR_OUTOFMEM, NULL); return NULL; }
#define INIT_PUB(t, p) (p)->pub.type = (t); (p)->pub.total = 0;
#define COPY_NODE(dst, src) dst->id = src->id; dst->data = src->data;
#define GET_MOD(num, mod) if ((num) < 0) (num) = -(-(num) % (mod)); else if ((num > 0)) (num) = ((num) % (mod));

//共有的数据头
typedef struct
{
	CRUINT8 type;  //用于表示据结构类型
	CRUINT64 total;  //用于表示现在有多少个元素
} CRSTRUCTUREPUB;

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
	CRINT64 id;
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
	IF_FAILED_QUIT(pInner);
	pInner->arr = malloc(sizeof(CRUINT8));
	if (!pInner->arr)
	{
		free(pInner);
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return NULL;
	}
	INIT_PUB(DYN, pInner);
	pInner->capacity = 1;
	return pInner;
}

CRAPI CRSTRUCTURE CRTree()
{
	PCRTREE pInner = malloc(sizeof(CRTREE));
	IF_FAILED_QUIT(pInner);
	INIT_PUB(TRE, pInner);
	pInner->root = NULL;
	return pInner;
}

CRAPI CRSTRUCTURE CRLinear()
{
	PCRLINEAR pInner = malloc(sizeof(CRLINEAR));
	IF_FAILED_QUIT(pInner);
	INIT_PUB(LIN, pInner);
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

CRAPI CRCODE CRDynPush(CRSTRUCTURE dyn, CRUINT8 data)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	if (pInner->pub.total < pInner->capacity)
	{
		pInner->arr[pInner->pub.total++] = data;
		return 0;
	}
	//需要扩容的情况，不得超过容量限制
	if (pInner->capacity >= DYN_MAX)
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		return CRERR_STRUCTURE_FULL;
	}
	pInner->capacity <<= 1;
	CRUINT8* tmp = _sizeup_cr_(pInner->arr, pInner->pub.total, pInner->capacity);
	if (!tmp)
	{
		CRThrowError(CRERR_STRUCTURE_RESIZE, CRDES_STRUCTURE_RESIZE);
		return CRERR_STRUCTURE_RESIZE;
	}
	tmp[pInner->pub.total++] = data;
	pInner->arr = tmp;
	return 0;
}

CRAPI CRCODE CRDynSet(CRSTRUCTURE dyn, CRUINT8 data, CRUINT32 sub)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	if (sub < pInner->pub.total)
		pInner->arr[sub] = data;
	else if (sub < pInner->capacity)
	{
		pInner->arr[sub] = data;
		pInner->pub.total = sub + 1;
	}
	else if (sub < DYN_MAX)
	{
		while (pInner->capacity < sub) pInner->capacity <<= 1;
		CRUINT8* tmp = _sizeup_cr_(pInner->arr, pInner->pub.total, pInner->capacity);
		if (!tmp)
			return CRERR_OUTOFMEM;
		tmp[sub++] = data;
		pInner->arr = tmp;
		pInner->pub.total = sub;
	}
	else
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		return CRERR_STRUCTURE_FULL;
	}
	return 0;
}

CRAPI CRCODE CRDynPop(CRSTRUCTURE dyn, CRUINT8* data)
{
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	if (pInner->pub.total == 0)
		return CRERR_NOTFOUND;
	pInner->pub.total--;
	*data = pInner->arr[pInner->pub.total];
	if (pInner->pub.total < pInner->capacity >> 1 && pInner->capacity > 32)//可以释放一些空间
	{
		pInner->capacity >>= 1;
		pInner->arr = _sizedown_cr_(pInner->arr, pInner->capacity);
	}
	return 0;
}

CRAPI CRCODE CRDynSeek(CRSTRUCTURE dyn, CRUINT8* data, CRUINT32 sub)
{
	PCRDYN pInner = dyn;
	if (pInner && pInner->pub.type == DYN && sub < pInner->pub.total)
	{
		*data = pInner->arr[sub];
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
	if (!pInner->pub.total)
	{
		CRThrowError(CRERR_NOTFOUND, NULL);
		return NULL;
	}
	CRUINT8* out = malloc(pInner->pub.total * sizeof(CRUINT8));
	IF_FAILED_QUIT(out);
	memcpy(out, pInner->arr, pInner->pub.total * sizeof(CRUINT8));
	*size = pInner->pub.total;
	return out;
}

//
//
//键值树的实现
//

PTREENODE _create_treenode_cr_(CRUINT64 id, CRLVOID data, PTREENODE parent)
{
	PTREENODE pNode = malloc(sizeof(TREENODE));
	if (pNode)
	{
		pNode->parent = parent;
		pNode->red = CRTRUE;
		pNode->id = id;
		pNode->data = data;
		pNode->left = NULL;
		pNode->right = NULL;
	}
	return pNode;
}

CRBOOL _left_node_(PTREENODE node)
{
	if (node && node->parent && node == node->parent->left)
		return CRTRUE;
	return CRFALSE;
}

PTREENODE _brother_(PTREENODE node)
{
	if (_left_node_(node))
		return node->parent->right;
	else if (node->parent)
		return node->parent->left;
	return NULL;
}

void _left_rotate_(PTREENODE node)
{
	if (!node->right)
		return;
	PTREENODE top = node, right = node->right, child = node->left;
	if (_left_node_(node))
		node->parent->left = right;
	else if (node->parent)
		node->parent->right = right;
	right->parent = node->parent;
	right->left = top;
	top->right = child;
	top->parent = right;
	if (child)
		child->parent = top;
}

void _right_rotate_(PTREENODE node)
{
	if (!node->left)
		return;
	PTREENODE top = node, left = node->left, child = left->right;
	if (_left_node_(node))
		node->parent->left = left;
	else if (node->parent)
		node->parent->right = left;
	left->parent = top->parent;
	left->right = top;
	top->left = child;
	top->parent = left;
	if (child)
		child->parent = top;
}

//吐槽：插入操作比删除操作简单多了
CRAPI CRCODE CRTreePut(CRSTRUCTURE tree, CRLVOID data, CRUINT64 id)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	if (!pInner->root)
	{
		pInner->root = _create_treenode_cr_(id, data, NULL);
		pInner->root->red = CRFALSE;
		goto Done;
	}
	PTREENODE node = pInner->root;
	while (1)
	{
		if (node->id == id)
		{
			node->data = data;
			return 0;
		}
		else if (node->id > id)
		{
			if (node->left) node = node->left;
			else
			{
				node->left = _create_treenode_cr_(id, data, node);
				if (!node->left) return CRERR_OUTOFMEM;
				node = node->left;
				break;
			}
		}
		else
		{
			if (node->right) node = node->right;
			else
			{
				node->right = _create_treenode_cr_(id, data, node);
				if (!node->right) return CRERR_OUTOFMEM;
				node = node->right;
				break;
			}
		}
	}
	//插入完毕，开始修正操作
Fix:
	if (!node->parent->red)
		goto Done;
	PTREENODE brother = _brother_(node->parent);
	if (brother && brother->red)
	{
		node->parent->red = CRFALSE;
		brother->red = CRFALSE;
		if (brother->parent == pInner->root)
			goto Done;
		brother->parent->red = CRTRUE;
		node = brother->parent;
		goto Fix;
	}
	if (node->parent->parent == pInner->root)
		pInner->root = node->parent;
	node->parent->parent->red = CRTRUE;
	if (_left_node_(node->parent))
	{
		if (_left_node_(node))
		{
			node->parent->red = CRFALSE;
			_right_rotate_(node->parent->parent);
		}
		else
		{
			node->red = CRFALSE;
			_left_rotate_(node->parent);
			_right_rotate_(node->parent);
		}
	}
	else
	{
		if (_left_node_(node))
		{
			node->parent->red = CRFALSE;
			_left_rotate_(node->parent->parent);
		}
		else
		{
			node->red = CRFALSE;
			_right_rotate_(node->parent);
			_left_rotate_(node->parent);
		}
	}
Done:
	pInner->pub.total++;
	return 0;
}

CRAPI CRCODE CRTreeSeek(CRSTRUCTURE tree, CRLVOID* data, CRUINT64 id)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	if (!pInner->root)
		return CRERR_NOTFOUND;
	PTREENODE node = pInner->root;
	while (1)
	{
		if (node->id == id)
			*data = node->data;
		return 0;
		if (node->id > id && node->left)
			node = node->left;
		else if (node->id < id && node->right)
			node = node->right;
		else break;
	}
	return CRERR_NOTFOUND;
}

//
//噩梦——删除红黑树节点

PTREENODE _prior_node_(PTREENODE node)
{
	/*
	* 此处不需要考虑向上寻找结点，因为在删除情况下不会用到这种情况
	* 在删除时，只要需要寻找前驱节点，就说明这个节点下方必然有两个节点
	*/
	node = node->left;
	while (node->right) node = node->right;
	return node;
}

void _fix_after_get_(PCRTREE pInner, PTREENODE node)
{
	PTREENODE brother = _brother_(node);
Fix:
	if (node->parent->parent)
	{
		brother->red = CRTRUE;
		if (node->parent->red)
			node->parent->red = CRFALSE;
		else
		{
			node = node->parent;
			goto Fix;
		}
	}
	else
	{
		pInner->root = brother;
		if (_left_node_(node))
		{
			if (brother->red)
			{
				node->parent->red = CRTRUE;
				brother->red = CRFALSE;
				_left_rotate_(node->parent);
			}
			else if (brother->right->red)
			{
				brother->right->red = CRFALSE;
				_left_rotate_(node->parent);
			}
			else if (brother->left->red)
			{
				pInner->root = brother->left;
				brother->left->red = CRFALSE;
				_right_rotate_(brother);
				_left_rotate_(node->parent);
			}
			else
			{
				pInner->root = node->parent;
				brother->red = CRTRUE;
			}
		}
		else
		{
			if (brother->red)
			{
				node->parent->red = CRTRUE;
				brother->red = CRFALSE;
				_right_rotate_(node->parent);
			}
			else if (brother->left->red)
			{
				brother->left->red = CRFALSE;
				_right_rotate_(node->parent);
			}
			else if (brother->right->red)
			{
				pInner->root = brother->right;
				brother->right->red = CRFALSE;
				_left_rotate_(brother);
				_right_rotate_(node->parent);
			}
			else
			{
				pInner->root = node->parent;
				brother->red = CRTRUE;
			}
		}
	}
}

void _fix_left_(PCRTREE pInner, PTREENODE node, PTREENODE brother)
{
	if (brother->red)
	{
		if (node->parent == pInner->root)
			pInner->root = brother;
		brother->red = CRFALSE;
		node->parent->red = CRTRUE;
		_left_rotate_(node->parent);
		brother = node->parent->right;
	}
	if (brother->right)
	{
		if (node->parent == pInner->root)
			pInner->root = brother;
		brother->red = node->parent->red;
		node->parent->red = CRFALSE;
		brother->right->red = CRFALSE;
		_left_rotate_(node->parent);
		node->parent->left = NULL;
		free(node);
	}
	else if (brother->left)
	{
		if (node->parent == pInner->root)
			pInner->root = brother->left;
		node->parent->red = CRFALSE;
		brother->left->red = node->parent->red;
		_right_rotate_(brother);
		_left_rotate_(brother->parent->parent);
		node->parent->left = NULL;
		free(node);
	}
	else
	{
		brother->red = CRTRUE;
		node->parent->left = NULL;
		free(node);
		if (brother->parent->red)
			brother->parent->red = CRFALSE;
		else
			if (brother->parent->parent)
				_fix_after_get_(pInner, brother->parent);
	}
}

void _fix_right_(PCRTREE pInner, PTREENODE node, PTREENODE brother)
{
	if (brother->red)
	{
		if (node->parent == pInner->root)
			pInner->root = brother;
		brother->red = CRFALSE;
		node->parent->red = CRTRUE;
		_right_rotate_(node->parent);
		brother = node->parent->left;
	}
	if (brother->left)
	{
		if (node->parent == pInner->root)
			pInner->root = brother;
		brother->red = node->parent->red;
		node->parent->red = CRFALSE;
		brother->left->red = CRFALSE;
		_right_rotate_(node->parent);
		node->parent->right = NULL;
		free(node);
	}
	else if (brother->right)
	{
		if (node->parent == pInner->root)
			pInner->root = brother->right;
		node->parent->red = CRFALSE;
		brother->right->red = node->parent->red;
		_left_rotate_(brother);
		_right_rotate_(brother->parent->parent);
		node->parent->right = NULL;
		free(node);
	}
	else
	{
		brother->red = CRTRUE;
		node->parent->right = NULL;
		free(node);
		if (brother->parent->red)
			brother->parent->red = CRFALSE;
		else
			if (brother->parent->parent)
				_fix_after_get_(pInner, brother->parent);
	}
}

CRAPI CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRUINT64 id)
{
	PCRTREE pInner = tree;
	if (!pInner || pInner->pub.type != TRE)
		return CRERR_INVALID;
	if (!pInner->root)
		return CRERR_NOTFOUND;
	PTREENODE node = pInner->root;
	while (1)
	{
		if (node->id == id)
			break;
		else if (node->id > id && node->left)
			node = node->left;
		else if (node->id < id && node->right)
			node = node->right;
		else
			return CRERR_NOTFOUND;
	}
	*data = node->data;
	//开始移除及修正
	if (node->left && node->right)
	{
		PTREENODE prior = _prior_node_(node);
		COPY_NODE(node, prior);
		node = prior;
	}
Fix:
	if (node->red)
	{
		if (_left_node_(node))
			node->parent->left = NULL;
		else node->parent->right = NULL;
		free(node);
		goto Done;
	}
	if (node->left)
	{
		COPY_NODE(node, node->left);
		free(node->left);
		node->left = NULL;
	}
	else if (node->right)
	{
		COPY_NODE(node, node->right);
		free(node->right);
		node->right = NULL;
	}
	else  //node已经是叶子节点了
	{
		if (node == pInner->root)
		{
			pInner->root = NULL;
			free(node);
			goto Done;
		}
		PTREENODE brother = _brother_(node);
		if (_left_node_(node))
			_fix_left_(pInner, node, brother);
		else
			_fix_right_(pInner, node, brother);
	}
Done:
	pInner->pub.total--;
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
	PLINEARNODE ins = malloc(sizeof(LINEARNODE));
	if (!ins) return CRERR_OUTOFMEM;
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
	return 0;
}

CRAPI CRCODE CRLinSeek(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek)
{
	PCRLINEAR pInner = lin;
	if (!pInner || pInner->pub.type != LIN)
		return CRERR_INVALID;
	if (!pInner->hook)
		return CRERR_NOTFOUND;
	PLINEARNODE node = pInner->hook;
	GET_MOD(seek, pInner->pub.total);
	if (seek < 0)
		while (seek < 0) { seek++; node = node->prior; }
	else
		while (seek > 0) { seek--; node = node->after; }
	*data = node->data;
	return 0;
}

CRAPI CRCODE CRLinGet(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek)
{
	PCRLINEAR pInner = lin;
	if (!pInner || pInner->pub.type != LIN)
		return CRERR_INVALID;
	if (!pInner->hook)
		return CRERR_NOTFOUND;
	PLINEARNODE node = pInner->hook;
	GET_MOD(seek, pInner->pub.total);
	if (seek < 0)
		while (seek < 0) { seek++; node = node->prior; }
	else
		while (seek > 0) { seek--; node = node->after; }
	*data = node->data;
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
	PLINEARNODE node = NULL;
	for (int i = 0; i < lin->pub.total; i++)
	{
		cal(lin->hook->data);
		node = lin->hook;
		lin->hook = lin->hook->after;
		free(node);
	}
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
	if (!cal)
		cal = _struc_do_nothing_;
	if (pub->type < 0 || pub->type > 3)
		return CRERR_INVALID;
	clearFuncs[pub->type](s, cal);
	free(s);
	return 0;
}