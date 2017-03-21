#pragma once

/*
* author:haohaosong
* date:2017/3/21
* note:空间配置器的模拟实现
*/

//定义一级空间配置器
template <int inst>
class __MallocAllocTemplate
{
private:
	//进行空间的申请
	static void *OOM_Malloc(size_t);

	static void *OOM_Realloc(void *, size_t);

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
	static void* Deallocate(void* p)
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
void(*__MallocAllocTemplate<inst>::SetMallocHander)() = 0;

template<int inst>
void* __MallocAllocTemplate<inst>::OOM_Malloc(size_t)
{
	//定义函数指针
	void(*my_malloc_realloc_hander)();
	//定义返回值
	void(*ret)();

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
