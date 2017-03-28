#pragma once

#include<iostream>
using namespace std;

#include"Trace.h"

//����һ���ռ�������
template <int inst>
class __MallocAllocTemplate
{
private:
	//���пռ�����룬����Malloc
	static void *OOM_Malloc(size_t);

	//����Realloc
	static void *OOM_Realloc(void *, size_t);

	//�Լ��������õľ������ָ��
	static void(*__MalloAlloc_OOM_Hander)();

public:
	//���пռ������
	static void* Allocate(size_t n)
	{
		//ֱ�ӵ���malloc�������ɹ�������OOM_Malloc
		void* ret = malloc(n);
		if (ret == NULL)
			ret = OOM_Malloc(n);

		return ret;
	}

	//���пռ���ͷ�
	static void Deallocate(void* p)
	{
		//ֱ�ӵ���free()
		free(p);
	}

	//���пռ�����·���
	static void* Reallocate(void* p,size_t newsize)
	{
		void* ret = realloc(p, newsize);
		if (ret == NULL)
			ret = OOM_Realloc(p, newsize);

		return ret;
	}
	
	//��������ռ�ľ��
	static void(*SetMallocHander(void* fun()))()
	{
		//���ص��Ǹı�֮ǰ�ĺ���ָ��
		void(*old)() = SetMallocHander;
		SetMallocHander = f;
		return old;
	}
};

//����Ϊ���麯��
//�Լ����Դ����Լ�����ĺ���
template<int inst>
void(*__MallocAllocTemplate<inst>::__MalloAlloc_OOM_Hander)() = 0;
//void(*__MallocAllocTemplate<inst>::SetMallocHander)() = 0;

template<int inst>
void* __MallocAllocTemplate<inst>::OOM_Malloc(size_t n)
{
	//���庯��ָ��
	void(*my_malloc_realloc_hander)();
	//���巵��ֵ
	void* ret;

	while (1)
	{
		//ע��:����Լ������__MallocAlloc_OOM_Hander�޷�����ռ�
		//��ᵼ����ѭ��
		my_malloc_realloc_hander = __MalloAlloc_OOM_Hander;
		if (my_malloc_realloc_hander == 0)
			throw("__THROW_BAD_ALLOC");

		//ͨ������ָ�����
		(*my_malloc_realloc_hander)();

		//�������룬������ɹ����򷵻�
		//���򣬽���ѭ��������������Ƭ��������ռ�
		ret = malloc(n);
		if (ret)
			return ret;
	}
}

//��OOM_Malloc()ԭ����һ����
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




//��������ռ�������
template <bool threads, int inst>
class __DefaultAllocTemplate
{
public:
	//�������γ���
	static const int __ALIGN = 8;//���л�׼ֵ
	static const int __MAX_BYTES = 128;//���ֵ
	static const int __NFREELISTS = __MAX_BYTES / __ALIGN;//���������С

	static char* startFree;//�ڴ��ˮλ�ߵĿ�ʼ
	static char* endFree;//�ڴ��ˮλ�ߵĽ���[
	static size_t heapSize;//��ϵͳ����ѵ��ܴ�С

	//���϶���
	//�Զ����뵽8���ֽ�
	static size_t ROUND_UP(size_t size) 
	{
		return (((size)+__ALIGN - 1) & ~(__ALIGN - 1));
	}
public:
	union obj 
	{
		//ָ����һ���ڵ��ָ��
		union obj * freeListLink;

		/* ���������ݲ���� */
		char client_data[1]; /* �ͻ�Ҫ������?? */
	};

	//������������
	static obj * freeList[__NFREELISTS];

	//������������Ӧ���±�
	//size > 0
	static  size_t FREELIST_INDEX(size_t size) 
	{
		return ((size- 1) / __ALIGN);
	}

	//���ڴ�ػ�ô���ڴ沢��������������
	static void *Refill(size_t size);

	//���ڴ���У�����ڴ棬nobjs���ڴ������ã���������ɹ��Ŀռ����
	static char *chunkAlloc(size_t size, int &nobjs);

public:
	static void* Allocate(size_t n)
	{
		__TRACE_DEBUG("�����ڴ�:%d\n", n);

		obj** myFreeList;
		obj* ret;

		//����128���ֽڣ�����һ���ռ�������
		if (n >= __MAX_BYTES)
			return __MallocAllocTemplate<0>::Allocate(n);

		size_t index = FREELIST_INDEX(n);
		myFreeList = freeList + index;
		ret = *myFreeList;
		if (ret == NULL)
		{
			//�����ǰλ��û�йҽڵ㣬�͵���Refill��������
			void * r = Refill(n);
			__TRACE_DEBUG("��������freeList[%d]ȡ�ڵ�\n", index);

			return r;
		}

		//����ͷɾ����ͷָ��ָ����һ���ڵ�
		*myFreeList = ret->freeListLink;
		return ret;
	}

	static void Deallocate(void* p, size_t n)
	{
		__TRACE_DEBUG("�ͷ��ڴ�(p:%p, n: %u)\n", p, n);

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

//���о�̬��Ա�ĳ�ʼ��
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
	size_t BytesLeft = (size_t)(endFree-startFree);//ʣ����ڴ�

	if (BytesLeft >= TotalBytes)
	{
		//����ȫ������
		__TRACE_DEBUG("�ڴ�����㹻�Ŀռ����%d������\n",nobjs);

		ret = startFree;
		startFree += TotalBytes;
		return ret;
	}
	else if (BytesLeft >= size)
	{
		__TRACE_DEBUG("�ڴ��ֻ���Է���%d������\n", nobjs);

		//���벻��20�飬����������1������
		nobjs = BytesLeft / size;//�������������ڴ�����
		TotalBytes = nobjs* size;
		ret = startFree;
		startFree += TotalBytes;
		return ret;
	}
	else
	{
		__TRACE_DEBUG("�ڴ�ط��䲻��1������\n", nobjs);

		//��1����ڴ涼ľ���ˣ���ϵͳ��������
		size_t BytesToGet = 2 * TotalBytes + ROUND_UP(heapSize >> 4);

		//����ڴ���л���ʣ�࣬��ʣ����ڴ������������Ķ�Ӧ�ڵ���
		if (BytesLeft > 0)
		{
			size_t index = FREELIST_INDEX(BytesLeft);
			obj** myFreeList = freeList + index;
			((obj*)startFree)->freeListLink = *myFreeList;
			*myFreeList = (obj*)startFree;

			__TRACE_DEBUG("�ڴ�ؽ�ʣ����ڴ����freeList[%d]��\n", index);
		}

		startFree = (char*)malloc(BytesToGet);
		__TRACE_DEBUG("��ϵͳ�л�ȡ%d�ڴ�\n", BytesToGet);

		if (startFree == NULL)
		{
			//��ϵͳ��û�����뵽�ڴ�
			//�Ӹ��������������ȥѰ��
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
			
			//��ֹ��һ���Malloc�׳��쳣
			__TRACE_DEBUG("���İ취��ȥһ���ռ��������н��в���\n");
			endFree = 0;
			__MallocAllocTemplate<0>::Allocate(BytesToGet); 
		}
		heapSize += BytesToGet;
		endFree = startFree + BytesToGet;
		return chunkAlloc(size,nobjs);
	}
}

//����ChunckAlloc�����ڴ棬����ӵ�����������
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

	//��������ڴ���뵽����������
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
		__TRACE_DEBUG("�����ڴ��%d��\n",i+1);
		v.push_back((int*)sa.Allocate());
	}

	int* p = (int*)sa.Allocate();

	int i = 1;
	while (!v.empty())
	{
		__TRACE_DEBUG("�ͷ��ڴ��%d��\n", i++);
		
		int* ptm = v.front();
		v.pop_back();
		sa.Deallocate(ptm);
	}

	sa.Deallocate(p);
}