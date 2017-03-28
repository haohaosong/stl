#pragma once

#include<iostream>
using namespace std;

#include"Trace.h"

//定义一级空间配置器
template <int inst>
class __MallocAllocTemplate
{
private:
	//进行空间的申请，调用Malloc
	static void *OOM_Malloc(size_t);

	//调用Realloc
	static void *OOM_Realloc(void *, size_t);

	//自己可以设置的句柄函数指针
	static void(*__MalloAlloc_OOM_Hander)();

public:
	//进行空间的申请
	static void* Allocate(size_t n)
	{
		//直接调用malloc，若不成功，调用OOM_Malloc
		void* ret = malloc(n);
		if (ret == NULL)
			ret = OOM_Malloc(n);

		return ret;
	}

	//进行空间的释放
	static void Deallocate(void* p)
	{
		//直接调用free()
		free(p);
	}

	//进行空间的重新分配
	static void* Reallocate(void* p,size_t newsize)
	{
		void* ret = realloc(p, newsize);
		if (ret == NULL)
			ret = OOM_Realloc(p, newsize);

		return ret;
	}
	
	//设置申请空间的句柄
	static void(*SetMallocHander(void* fun()))()
	{
		//返回的是改变之前的函数指针
		void(*old)() = SetMallocHander;
		SetMallocHander = f;
		return old;
	}
};

//定义为纯虚函数
//自己可以传入自己定义的函数
template<int inst>
void(*__MallocAllocTemplate<inst>::__MalloAlloc_OOM_Hander)() = 0;
//void(*__MallocAllocTemplate<inst>::SetMallocHander)() = 0;

template<int inst>
void* __MallocAllocTemplate<inst>::OOM_Malloc(size_t n)
{
	//定义函数指针
	void(*my_malloc_realloc_hander)();
	//定义返回值
	void* ret;

	while (1)
	{
		//注意:如果自己定义的__MallocAlloc_OOM_Hander无法整理空间
		//则会导致死循环
		my_malloc_realloc_hander = __MalloAlloc_OOM_Hander;
		if (my_malloc_realloc_hander == 0)
			throw("__THROW_BAD_ALLOC");

		//通过函数指针调用
		(*my_malloc_realloc_hander)();

		//进行申请，若申请成功，则返回
		//否则，进行循环，重新整理碎片，再申请空间
		ret = malloc(n);
		if (ret)
			return ret;
	}
}

//和OOM_Malloc()原理是一样的
template<int inst>
void* __MallocAllocTemplate<inst>::OOM_Realloc(void* p, size_t n)
{
	void(*my_malloc_realloc_hander);
	void* ret;

	while (1)
	{
		my_malloc_realloc_hander = __MalloAlloc_OOM_Hander;
		if (my_malloc_realloc_hander == 0)
			throw("__THROW_BAD_ALLOC");

		(*my_malloc_realloc_hander)();
		ret = realloc(p, newsize);
		if (ret)
			return ret;
	}
}




//定义二级空间配置器
template <bool threads, int inst>
class __DefaultAllocTemplate
{
public:
	//定义整形常量
	static const int __ALIGN = 8;//排列基准值
	static const int __MAX_BYTES = 128;//最大值
	static const int __NFREELISTS = __MAX_BYTES / __ALIGN;//自由链表大小

	static char* startFree;//内存池水位线的开始
	static char* endFree;//内存池水位线的结束[
	static size_t heapSize;//从系统分配堆的总大小

	//向上对齐
	//自动对齐到8个字节
	static size_t ROUND_UP(size_t size) 
	{
		return (((size)+__ALIGN - 1) & ~(__ALIGN - 1));
	}
public:
	union obj 
	{
		//指向下一个节点的指针
		union obj * freeListLink;

		/* 这条语句可暂不理解 */
		char client_data[1]; /* 客户要看见他?? */
	};

	//定义自由链表
	static obj * freeList[__NFREELISTS];

	//求出自由链表对应的下标
	//size > 0
	static  size_t FREELIST_INDEX(size_t size) 
	{
		return ((size- 1) / __ALIGN);
	}

	//从内存池获得大块内存并链到自由链表中
	static void *Refill(size_t size);

	//从内存池中，获得内存，nobjs由于传的引用，代表申请成功的空间个数
	static char *chunkAlloc(size_t size, int &nobjs);

public:
	static void* Allocate(size_t n)
	{
		__TRACE_DEBUG("请求内存:%d\n", n);

		obj** myFreeList;
		obj* ret;

		//大于128个字节，调用一级空间配置器
		if (n >= __MAX_BYTES)
			return __MallocAllocTemplate<0>::Allocate(n);

		size_t index = FREELIST_INDEX(n);
		myFreeList = freeList + index;
		ret = *myFreeList;
		if (ret == NULL)
		{
			//如果当前位置没有挂节点，就调用Refill进行申请
			void * r = Refill(n);
			__TRACE_DEBUG("自由链表freeList[%d]取节点\n", index);

			return r;
		}

		//进行头删，将头指针指向下一个节点
		*myFreeList = ret->freeListLink;
		return ret;
	}

	static void Deallocate(void* p, size_t n)
	{
		__TRACE_DEBUG("释放内存(p:%p, n: %u)\n", p, n);

		obj** myFreeList;
		obj* q = (obj*)p;

		if (n > __MAX_BYTES)
		{
			__MallocAllocTemplate<0>::Deallocate(p/*, n*/);
			return ;
		}

		myFreeList = freeList + FREELIST_INDEX(n);
		q->freeListLink = *myFreeList;
		*myFreeList = q;
	}

	static void* Reallocate(void* p, size_t oldSize,size_t newSize);
};

//进行静态成员的初始化
template <bool threads, int inst>
typename __DefaultAllocTemplate<threads, inst>::obj*\
__DefaultAllocTemplate<threads, inst>::freeList[__DefaultAllocTemplate<threads, inst>::__NFREELISTS] = {0};

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::startFree = NULL;

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::endFree = NULL;

template <bool threads, int inst>
size_t __DefaultAllocTemplate<threads, inst>::heapSize = 0;

template<bool threads,int inst>
char* __DefaultAllocTemplate<threads, inst>::chunkAlloc(size_t size,int& nobjs)
{
	char* ret;
	size_t TotalBytes = size* nobjs;
	size_t BytesLeft = (size_t)(endFree-startFree);//剩余的内存

	if (BytesLeft >= TotalBytes)
	{
		//可以全部申请
		__TRACE_DEBUG("内存池有足够的空间分配%d个对象\n",nobjs);

		ret = startFree;
		startFree += TotalBytes;
		return ret;
	}
	else if (BytesLeft >= size)
	{
		__TRACE_DEBUG("内存池只可以分配%d个对象\n", nobjs);

		//申请不到20块，但可以申请1块以上
		nobjs = BytesLeft / size;//求出可以申请的内存块个数
		TotalBytes = nobjs* size;
		ret = startFree;
		startFree += TotalBytes;
		return ret;
	}
	else
	{
		__TRACE_DEBUG("内存池分配不了1个对象\n", nobjs);

		//连1块的内存都木有了，向系统进行申请
		size_t BytesToGet = 2 * TotalBytes + ROUND_UP(heapSize >> 4);

		//如果内存池中还有剩余，则将剩余的内存放入自由链表的对应节点中
		if (BytesLeft > 0)
		{
			size_t index = FREELIST_INDEX(BytesLeft);
			obj** myFreeList = freeList + index;
			((obj*)startFree)->freeListLink = *myFreeList;
			*myFreeList = (obj*)startFree;

			__TRACE_DEBUG("内存池将剩余的内存放入freeList[%d]中\n", index);
		}

		startFree = (char*)malloc(BytesToGet);
		__TRACE_DEBUG("从系统中获取%d内存\n", BytesToGet);

		if (startFree == NULL)
		{
			//从系统中没有申请到内存
			//从更大的自由链表中去寻找
			int i;
			obj** myFreeList;
			obj** p;
			for (i = size; i < __MAX_BYTES; i += __ALIGN)
			{
				myFreeList = freeList + FREELIST_INDEX(i);
				p = myFreeList;
				if (p != NULL)
				{
					*myFreeList = (*p)->freeListLink;
					startFree = (char*)p;
					endFree = startFree + i;
					return chunkAlloc(size, nobjs);
				}
			}
			
			//防止下一句的Malloc抛出异常
			__TRACE_DEBUG("最后的办法，去一级空间配置器中进行查找\n");
			endFree = 0;
			__MallocAllocTemplate<0>::Allocate(BytesToGet); 
		}
		heapSize += BytesToGet;
		endFree = startFree + BytesToGet;
		return chunkAlloc(size,nobjs);
	}
}

//调用ChunckAlloc申请内存，并添加到自由链表中
template<bool threads,int inst>
void* __DefaultAllocTemplate<threads, inst>::Refill(size_t size)
{
	int nobjs = 20;
	obj ** myFreeList;
	void* ret;
	char* chunck = chunkAlloc(size, nobjs);

	if (nobjs == 1)
		return chunck;

	size_t index = FREELIST_INDEX(size);
	myFreeList = freeList + index;

	//将申请的内存加入到自由链表中
	ret = chunck;
	obj* cur = (obj*)(chunck + size);
	freeList[index] = cur;
	for (int i = 2; i<nobjs; ++i)
	{
		cur->freeListLink = (obj*)(chunck + size*i);
		cur = cur->freeListLink;
	}
	cur->freeListLink = NULL;
	return ret;
}

template<bool threads,int inst>
void* __DefaultAllocTemplate<threads, inst>::Reallocate(void*p, size_t oldSize,size_t newSize)
{
	void* ret;
	size_t cpSize;

	if (oldSize > __MAX_BYTES && newSize < __MAX_BYTES)
		return Reallocate(p,newSize);

	if (ROUND_UP(oldSize) == ROUND_UP(newSize))
		return p;

	ret = Allocate(new_sz);
	copySize = newSize > oldSize ? oldSize : newSize;
	memcpy(result, p, copySize);
	Deallocate(p, oldSize);
	return(ret);
}

typedef __DefaultAllocTemplate<0, 0> _alloc;

template<class T, class Alloc = _alloc>
class SimpleAlloc 
{
public:
	static T* Allocate(size_t n)
	{
		return 0 == n ? 0 : (T*)Alloc::Allocate(n * sizeof (T));
	}
	static T* Allocate(void)
	{
		return (T*)Alloc::Allocate(sizeof (T));
	}
	static void Deallocate(T *p, size_t n)
	{
		if (0 != n) Alloc::Deallocate(p, n * sizeof (T));
	}
	static void Deallocate(T *p)
	{
		Alloc::Deallocate(p, sizeof (T));
	}
};

#include<vector>

void TestAlloc()
{
	vector<int*> v;
	SimpleAlloc<int> sa;
	for (size_t i = 0; i < 20; ++i)
	{
		__TRACE_DEBUG("申请内存第%d个\n",i+1);
		v.push_back((int*)sa.Allocate());
	}

	int* p = (int*)sa.Allocate();

	int i = 1;
	while (!v.empty())
	{
		__TRACE_DEBUG("释放内存第%d个\n", i++);
		
		int* ptm = v.front();
		v.pop_back();
		sa.Deallocate(ptm);
	}

	sa.Deallocate(p);
}