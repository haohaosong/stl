#pragma once

/*
* author:haohaosong
* date:2017/4/21
* note:�����ڴ�ص�ʵ��
*/

#include<iostream>
using namespace std;

template<typename T>
class ObjPool
{
	//�������ڴ�Ľڵ�
	struct Node
	{
		Node* _mem;//ָ�����ڴ��ָ��
		size_t _n; //��ǰ�ڵ�����Ķ��������
		Node* _next;//ָ����һ����ڴ��

		Node(size_t nobjs)
		{
			_n = nobjs;
			_mem = (Node*)::operator new(_n*GetObjSize());//���ܵ��ù��캯��������ᵼ�����޵ݹ�
			_next = NULL;
		}

		~Node()
		{
			_n = 0;
			::operator delete(_mem);//ͬ��
			_mem = NULL;
			_next = NULL;
		}
	};
public:
	ObjPool(size_t initNobjs = 16, size_t maxNobjs = 1024)//���캯��
		:_initObj(initNobjs)
		, _maxObj(maxNobjs)
		,_useCount(0)
		,_lastUse(NULL)
	{
		_head = _tail = new Node(initNobjs);
	}

	~ObjPool()
	{
		Node* cur = _head;
		while (cur)
		{
			Node* next = cur->_next;
			delete cur;
			cur = next;
		}
		_head = _tail = NULL;
	}

	inline static size_t GetObjSize()
	{
		return sizeof(T) > sizeof(T*) ? sizeof(T) : sizeof(T*);
	}

	void* Allocate()
	{
		//����ʹ������ͷŵ��ڴ��
		if (_lastUse)
		{
			void* obj = _lastUse;
			_lastUse = *(T**)_lastUse;
			printf("ʹ������ͷŵ��ڴ��:%p\n", obj);
			return obj;
		}

		//��ʾû���ڴ������
		if (_useCount >= _tail->_n)
		{
			AllocNewNode();
			printf("û�п����ڴ�,ȥ����\n");
		}

		//��ʾ�п��õ��ڴ�
		void* obj = (char*)_tail->_mem + _useCount*GetObjSize();
		_useCount++;
		printf("�п��õ��ڴ��:%p\n", obj);
		return obj;
	}

	void Deallocate(void* ptr)
	{
		if (ptr)
		{
			printf("�ͷ��ڴ��:%p\n", ptr);
			*(T**)ptr = _lastUse;
			_lastUse = (T*)ptr;
		}
	}

	template<typename Val>
	T* New(const Val& val)
	{
		void* obj = Allocate();
		return new(obj)T(Val);
	}

	void Delete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			Deallocate();
		}
	}
protected:
	void AllocNewNode()//��ϵͳ���������ڴ�ڵ�
	{
		size_t n = _tail->_n * 2;
		if (n > _maxObj)
			n = _maxObj;

		Node* newNode = new Node(n);
		_tail->_next = newNode;
		_tail = newNode;
		_useCount = 0;
	}

protected:
	size_t _initObj;//��Ҫ��������ڴ��ĸ���
	size_t _maxObj;//�����������ڴ��ĸ���
	Node* _head;//ָ���ڴ�ص�ͷ�ڵ�
	Node* _tail;//ָ���ڴ�ص�β�ڵ�
	size_t _useCount;//��ǰ����ʹ�õĽڵ����
	T* _lastUse;//����ͷŵ��ڴ��
};

void TestObjPool()
{
	ObjPool<string> pool;             //�����ڴ�ض���
	string* p1 = (string*)pool.Allocate();//ʹ��һ������
	string* p2 = (string*)pool.Allocate();//ʹ��һ������
	pool.Deallocate(p1);                 //�ͷŶ���
	pool.Deallocate(p2);                 //�ͷŶ���
	string* p3 = (string*)pool.Allocate();//ʹ������ͷŵĶ���
	string* p4 = (string*)pool.Allocate();//ʹ������ͷŵĶ���
}