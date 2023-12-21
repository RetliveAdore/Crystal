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

typedef struct quaditems
{
	CRUINT8 subinf;  //为0表示无重叠，使用data，为1表示有重叠，使用dyn
	union
	{
		CRLVOID data;
		CRSTRUCTURE dyn;
	};
	CRUINT64 key;
	CRRECTU range;
}CRQUADITEMS, *PCRQUADITEMS;

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
	PCRQUADITEMS itemList;
	CRUINT32 listSize;  //表示现在itemList里面有效的元素有多少个
}QUADTREENODE, *PQUADTREENODE;

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
	node->itemList = NULL;
	//
	node->range.left = range.left;
	node->range.top = range.top;
	node->range.right = range.right;
	node->range.bottom = range.bottom;
	return node;
}

/*
* 假如返回CRFALSE，就表明不可再分（拆分到单个像素了）或者范围有误
* 分裂不仅会拆分区域，还会将元素分配到子区域
* 对于那种刚好在边界横跨两个区域的元素，就留在父节点
* 
* 当然，在插入阶段，那种范围重合的元素，会被归类为特殊，放在一起，
* 然后只占用一个一个元素的位置。
* 
* 拆分时会检查拆分后的结点是否需要继续拆分（少见的情况）
* 满足拆分条件的进一步拆分，直到拆到满足数量条件或单个像素
*/
CRBOOL _quad_split_(PQUADTREENODE node)
{
	return CRTRUE;
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
		pInner->root = malloc(sizeof(QUADTREENODE));
		if (!pInner->root)
		{
			DeleteCriticalSection(&pInner->pub.cs);
			free(pInner);
			CRThrowError(CRERR_OUTOFMEM, NULL);
			return NULL;
		}
		//必需初始化根节点
		pInner->root->itemList = calloc(pInner->maxItem, sizeof(CRQUADITEMS));
		pInner->root->LT = NULL;
		pInner->root->LB = NULL;
		pInner->root->RT = NULL;
		pInner->root->RB = NULL;
		//这是根范围，以有用户设置为准，通常为窗口大小
		//但是窗口大小经常变化的话，建议使用一个固定的值，然后计算映射
		pInner->root->range.left = 0;
		pInner->root->range.top = 0;
		pInner->root->range.right = w;
		pInner->root->range.bottom = h;
		return pInner;
	}
	CRThrowError(CRERR_OUTOFMEM, NULL);
	return NULL;
}

CRAPI CRCODE CRQuadtreePushin(CRTREEXTRA tree, CRRECTU range, CRUINT32 key)
{
	PCRQUADINNER pInner = tree;
	if (!pInner || pInner->pub.type != QUA)
		return CRERR_INVALID;

	return 0;
}

//十分经典的递归处理，不必多言
//即使从理论上来讲，最大递归深度也无法超过64层或32层，因为整型的最大值限制了这一点
void _free_quad_node_(PQUADTREENODE node, DSCallback cbk)
{
	if (node->LT)
		_free_quad_node_(node->LT, cbk);
	if (node->RT)
		_free_quad_node_(node->RT, cbk);
	if (node->LB)
		_free_quad_node_(node->LB, cbk);
	if (node->RB)
		_free_quad_node_(node->RB, cbk);
	if (node->itemList)
	{
		for (int i = 0; i < node->listSize; i++)
		{
			if (node->itemList[i].subinf)  //不为0表示是动态数组
			{
				CRFreeStructure(node->itemList[i].dyn, cbk);
			}
			else
			{
				if (cbk)
					cbk(node->itemList[i].data);
			}
		}
	}
	free(node->itemList);
	free(node);
}

void _free_quad_(PCRQUADINNER quad, DSCallback cbk)
{
	_free_quad_node_(quad->root, cbk);
	free(quad);
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