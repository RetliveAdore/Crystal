#include <Crystal.h>
#include <parts/CrTreeExtra.h>
#include <crerrors.h>

//必备多线程安全控制，要用到一些多线程的锁
#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#include <malloc.h>
#endif

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

#define CRQUAD_N 0x00
#define CRQUAD_L 0x01
#define CRQUAD_R 0x02
#define CRQUAD_T 0x04
#define CRQUAD_B 0x08

/*
* 这个结构和core中的数据头一模一样
* 但是不推荐使用core中的函数来查询容量
* 因为这样会很乱
*/

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

typedef struct quadtreenode
{
	CRPOINTU point;
	struct quadtreenode* LT;  //left top
	struct quadtreenode* RT;  //right top
	struct quadtreenode* LB;  //left bottom
	struct quadtreenode* RB;  //right bottom
	struct quadtreenode* parent;  //parent
	CRRECTU range;
	//存储每个元素的信息
	CRSTRUCTURE items;  //此数组用于存储键值以及覆盖范围结构体的堆指针
}QUADTREENODE, *PQUADTREENODE;

//一个具体，准确的空间实体
typedef struct quaditems
{
	CRLVOID data;
	CRRECTU cover;
	PQUADTREENODE hook;
}CRQUADITEMS, *PCRQUADITEMS;

typedef struct
{
	CRSTRUCTUREPUB pub;
	PQUADTREENODE root;
	CRUINT8 maxItem;
	CRSTRUCTURE keyTree;  //要想实现快速删除，就需要快速查找，通过ID找到范围进而找到对应的结点
}CRQUADINNER, *PCRQUADINNER;

//创建一个空的四叉树结点
static PQUADTREENODE _create_node_(CRRECTU range, PQUADTREENODE parent)
{
	PQUADTREENODE node = malloc(sizeof(QUADTREENODE));
	if (!node)
		return NULL;
	node->LT = NULL;
	node->LB = NULL;
	node->RT = NULL;
	node->RB = NULL;
	node->parent = parent;
	node->items = CRDynamicPtr();
	//
	node->range.left = range.left;
	node->range.top = range.top;
	node->range.right = range.right;
	node->range.bottom = range.bottom;
	return node;
}

/*定位函数
* 此函数将告诉你以当前的cover处于base的哪个部分
* 不仅可以处于单个区域，还可以跨区
*/
CRUINT8 _quad_partition_(CRRECTU cover, CRRECTU base, CRBOOL* crossed)
{
	CRUINT8 partition = CRQUAD_N;
	if (crossed) *crossed = CRFALSE;
	CRUINT32 x_m = base.left + (base.right - base.left) / 2;
	CRUINT32 y_m = base.top + (base.bottom - base.top) / 2;
	if (cover.right <= x_m)
		partition |= CRQUAD_L;
	else if (cover.left > x_m)
		partition |= CRQUAD_R;
	else
	{
		if (crossed) *crossed = CRTRUE;
		partition |= CRQUAD_L | CRQUAD_R;
	}
	if (cover.bottom <= y_m)
		partition |= CRQUAD_T;
	else if (cover.top > y_m)
		partition |= CRQUAD_B;
	else
	{
		if (crossed) *crossed = CRTRUE;
		partition |= CRQUAD_T | CRQUAD_B;
	}
	return partition;
}

/*
* 假如返回CRFALSE，就表明不可再分（拆分到单个像素了）或者范围有误
* 分裂不仅会拆分区域，还会将元素分配到子区域
* 对于那种刚好在边界横跨两个区域的元素，就留在父节点
* 
* 拆分时会检查拆分后的结点是否需要继续拆分（少见的情况）
* 满足拆分条件的进一步拆分，直到拆到满足数量条件或全部跨区无法拆分
*/
CRBOOL _quad_split_(PQUADTREENODE node)
{
	if (node->range.right - node->range.left <= 1 || node->range.bottom - node->range.top <= 1)
		return CRFALSE;
	//先对已有元素进行分区判断在决定是否分裂结点
	CRUINT32 cross = 0;  //统计跨区元素数量，只要不是全部跨区且结点可分裂，就有必要分裂结点
	PCRQUADITEMS titem;
	CRBOOL crossed;
	//
	for (int i = 0; i < CRStructureSize(node->items); i++)
	{
		CRDynSeekPtr(node->items, (CRLVOID*)&titem, i);
		_quad_partition_(titem->cover, node->range, &crossed);
		if (crossed) cross++;
	}
	if (cross >= CRStructureSize(node->items))
		return CRFALSE;
	//确定要分裂结点了
	CRRECTU range;
	CRUINT64 x_m = node->range.left + (node->range.right - node->range.left) / 2;
	CRUINT64 y_m = node->range.top + (node->range.bottom - node->range.top) / 2;
	range.left = node->range.left;
	range.top = node->range.top;
	range.right = x_m;
	range.bottom = y_m;
	node->LT = _create_node_(range, node);
	//
	range.left = x_m + 1;
	range.right = node->range.right;
	node->RT = _create_node_(range, node);
	//
	range.top = y_m + 1;
	range.bottom = node->range.bottom;
	node->RB = _create_node_(range, node);
	//
	range.left = node->range.left;
	range.right = x_m;
	node->LB = _create_node_(range, node);
	//使用新的dynPtr来承接新的item列表，就列表pop完毕之后销毁
	CRSTRUCTURE tmp = node->items;
	node->items = CRDynamicPtr();  //即使内存申请失败也不处理了，太繁琐了
	CRUINT8 p;
	while (!CRDynPopPtr(tmp, (CRLVOID*)&titem))
	{
		p = _quad_partition_(titem->cover, node->range, &crossed);
		if (crossed)
			CRDynPushPtr(node->items, titem);
		else
		{
			switch (p)
			{
			case CRQUAD_L | CRQUAD_T:
				if (CRDynPushPtr(node->LT->items, titem))
					free(titem);
				else titem->hook = node->LT;
				break;
			case CRQUAD_R | CRQUAD_T:
				if (CRDynPushPtr(node->RT->items, titem))
					free(titem);
				else titem->hook = node->RT;
				break;
			case CRQUAD_L | CRQUAD_B:
				if (CRDynPushPtr(node->RB->items, titem))
					free(titem);
				else titem->hook = node->RB;
				break;
			case CRQUAD_R | CRQUAD_B:
				if (CRDynPushPtr(node->LB->items, titem))
					free(titem);
				else titem->hook = node->LB;
				break;
			default:
				free(titem);
				break;
			}
		}
	}
	//
	CRFreeStructure(tmp, NULL);
	return CRTRUE;
}

void _fix_quadnode_split_(PQUADTREENODE node, CRUINT8 max)
{
	if (CRStructureSize(node->items) <= max || node->LT)
		return;
	if (_quad_split_(node))
	{
		_fix_quadnode_split_(node->LT, max);
		_fix_quadnode_split_(node->RT, max);
		_fix_quadnode_split_(node->RB, max);
		_fix_quadnode_split_(node->LB, max);
	}
}

//暂时不改变四叉树的结构
void _fix_quadnode_merge_(PQUADTREENODE node, CRUINT8 max)
{
	//递归处理，检查这下面一系列结点是否可以融合
	if (node->LT)
	{
		_fix_quadnode_merge_(node->LT, max);
		_fix_quadnode_merge_(node->RT, max);
		_fix_quadnode_merge_(node->RB, max);
		_fix_quadnode_merge_(node->LB, max);
	}
Merge:  //对当前结点做出判断，符合条件的进行融合
	if (node->parent)
	{
		node = node->parent;
	}
	else if (node->LT)
	{
		CRUINT32 numLT = CRStructureSize(node->LT->items);
		CRUINT32 numRT = CRStructureSize(node->RT->items);
		CRUINT32 numRB = CRStructureSize(node->RB->items);
		CRUINT32 numLB = CRStructureSize(node->LB->items);
		CRUINT32 numME = CRStructureSize(node->items);
		if (!numLT || !numRT || !numRB || !numLB)
			goto Melt;
		else if (numLT + numRT + numRB + numLB + numME <= max)
		{
			PCRQUADITEMS item;
			while (!CRDynPopPtr(node->LT->items, (CRLVOID*)&item)) CRDynPushPtr(node->items, item);
			while (!CRDynPopPtr(node->RT->items, (CRLVOID*)&item)) CRDynPushPtr(node->items, item);
			while (!CRDynPopPtr(node->RB->items, (CRLVOID*)&item)) CRDynPushPtr(node->items, item);
			while (!CRDynPopPtr(node->LB->items, (CRLVOID*)&item)) CRDynPushPtr(node->items, item);
			goto Melt;
		}
	}
	return;
Melt:
	CRFreeStructure(node->LT->items, NULL);
	free(node->LT);
	node->LT = NULL;
	CRFreeStructure(node->RT->items, NULL);
	free(node->RT);
	node->RT = NULL;
	CRFreeStructure(node->RB->items, NULL);
	free(node->RB);
	node->RB = NULL;
	CRFreeStructure(node->LB->items, NULL);
	free(node->LB);
	node->LB = NULL;
}

void _cover_clear_cbk_(CRLVOID data)
{
	free((PCRQUADITEMS)data);
}

//十分经典的递归处理，不必多言
//即使从理论上来讲，最大递归深度也无法超过64层或32层，因为整型的最大值限制了这一点
void _free_quad_node_(PQUADTREENODE node)
{
	if (node->LT)
		_free_quad_node_(node->LT);
	if (node->RT)
		_free_quad_node_(node->RT);
	if (node->LB)
		_free_quad_node_(node->LB);
	if (node->RB)
		_free_quad_node_(node->RB);
	CRFreeStructure(node->items, _cover_clear_cbk_);
	free(node);
}

void _free_quad_(PCRQUADINNER quad, DSCallback cbk)
{
	CRFreeStructure(quad->keyTree, cbk);
	_free_quad_node_(quad->root);
	free(quad);
}

CRAPI CRTREEXTRA CRQuadtree(CRUINT64 w, CRUINT64 h, CRUINT8 max)
{
	if (max == 0)
		max = 4;
	PCRQUADINNER pInner = malloc(sizeof(CRQUADINNER));
	if (pInner)
	{
		pInner->pub.total = 0;
		pInner->pub.type = QUA;
		pInner->maxItem = max;
		InitializeCriticalSection(&(pInner->pub.cs));
		CRRECTU range;
		range.left = 0;
		range.right = w;
		range.top = 0;
		range.bottom = h;
		pInner->root = _create_node_(range, NULL);
		pInner->keyTree = CRTree();
		if (!pInner->root)
		{
			DeleteCriticalSection(&pInner->pub.cs);
			free(pInner);
			CRThrowError(CRERR_OUTOFMEM, NULL);
			return NULL;
		}
		return pInner;
	}
	CRThrowError(CRERR_OUTOFMEM, NULL);
	return NULL;
}

CRAPI CRCODE CRQuadtreePushin(CRTREEXTRA tree, CRRECTU range, CRLVOID key)
{
	PCRQUADINNER pInner = tree;
	if (!pInner || pInner->pub.type != QUA)
		return CRERR_INVALID;
	PCRQUADITEMS item = malloc(sizeof(CRQUADITEMS));
	if (!item)
		return CRERR_OUTOFMEM;
	//
	item->cover.left = range.left;
	item->cover.top = range.top;
	item->cover.right = range.right;
	item->cover.bottom = range.bottom;
	item->data = key;
	//寻找item的所属结点
	CRBOOL crossed;
	CRUINT8 part;
	PQUADTREENODE node = pInner->root;
	while (1)
	{
		part = _quad_partition_(item->cover, node->range, &crossed);
		if (crossed)
			break;
		if (node->LT)  //只要有一个子节点，就说明另外三个也在
		{
			switch (part)
			{
			case CRQUAD_L | CRQUAD_T:
				node = node->LT;
				break;
			case CRQUAD_R | CRQUAD_T:
				node = node->RT;
				break;
			case CRQUAD_R | CRQUAD_B:
				node = node->RB;
				break;
			case CRQUAD_L | CRQUAD_B:
				node = node->LB;
				break;
			default:
				break;
			}
		}
		else
			break;
	}
	item->hook = node;
	CRDynPushPtr(node->items, item);
	CRTreePut(pInner->keyTree, item, (CRUINT64)key);
	//开始检查结点是否可调整
	_fix_quadnode_split_(node, pInner->maxItem);
	return 0;
}

//通过键值树直接找到此元素，然后从结点中移除此元素
//最后检查临近结点包括父节点的元素分布，满足数量的就融合结点
CRAPI CRCODE CRQuadtreeRemove(CRTREEXTRA tree, CRLVOID key)
{
	PCRQUADINNER pInner = tree;
	if (!pInner || pInner->pub.type != QUA)
		return CRERR_INVALID;
	PCRQUADITEMS item;
	if (CRTreeGet(pInner->keyTree, (CRLVOID*)&item, (CRUINT64)key))
		return CRERR_NOTFOUND;
	//命中结点之后开始移除并调整
	PQUADTREENODE node = item->hook;
	CRSTRUCTURE tmp = CRDynamicPtr();
	while (!CRDynPopPtr(node->items, (CRLVOID*)&item))
	{
		if (item->data != key)
			CRDynPushPtr(tmp, item);
		else free(item);
	}
	CRFreeStructure(node->items, NULL);
	node->items = tmp;
	_fix_quadnode_merge_(node, pInner->maxItem);
	return 0;
}

CRAPI CRCODE CRQuadtreeSearch(CRTREEXTRA tree, CRPOINTU p, CRSTRUCTURE dynPtrOut)
{
	PCRQUADINNER pInner = tree;
	if (!pInner || pInner->pub.type != QUA)
		return CRERR_INVALID;
	PQUADTREENODE node = pInner->root;
	if (p.x < pInner->root->range.left
		|| p.x > pInner->root->range.right
		|| p.y < pInner->root->range.top
		|| p.y > pInner->root->range.bottom)
		return 0;
	PCRQUADITEMS item;
	CRUINT32 x_m, y_m;
	while (1)
	{
		for (int i = 0; i < CRStructureSize(node->items); i++)
		{
			CRDynSeekPtr(node->items, (CRLVOID*)&item, i);
			if (p.x >= item->cover.left
				&& p.x <= item->cover.right
				&& p.y >= item->cover.top
				&& p.y <= item->cover.bottom)
				CRDynPushPtr(dynPtrOut, item->data);
		}
		x_m = node->range.left + (node->range.right - node->range.left) / 2;
		y_m = node->range.top + (node->range.bottom - node->range.top) / 2;
		if (node->LT)
		{
			if (p.x <= x_m && p.y <= y_m)
				node = node->LT;
			else if (p.x > x_m && p.y <= y_m)
				node = node->RT;
			else if (p.x > x_m && p.y > y_m)
				node = node->RB;
			else if (p.x <= x_m && p.y > y_m)
				node = node->LB;
		}
		else return 0;
	}
	return 0;
}

CRAPI CRCODE CRFreeTreextra(CRTREEXTRA* pTree, DSCallback cbk)
{
	CRSTRUCTUREPUB* pInner = *pTree;
	if (!pInner)
		return CRERR_INVALID;
	switch (pInner->type)
	{
	case QUA:
		_free_quad_((PCRQUADINNER)pInner, cbk);
		break;
	default:
		return CRERR_INVALID;
	}
	*pTree = NULL;
	return 0;
}