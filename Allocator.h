#pragma once

/*
* author:haohaosong
* date:2017/3/21
* note:�ռ���������ģ��ʵ��
*/

//����һ���ռ�������
template <int inst>
class __MallocAllocTemplate
{
private:
	//���пռ������
	static void *OOM_Malloc(size_t);

	static void *OOM_Realloc(void *, size_t);

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
	static void* Deallocate(void* p)
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
void(*__MallocAllocTemplate<inst>::SetMallocHander)() = 0;

template<int inst>
void* __MallocAllocTemplate<inst>::OOM_Malloc(size_t)
{
	//���庯��ָ��
	void(*my_malloc_realloc_hander)();
	//���巵��ֵ
	void(*ret)();

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
