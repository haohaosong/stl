#pragma once

/*
* author:haohaosong
* date:2017/4/21
* note:简易内存池的实现
*/

#include<iostream>
using namespace std;

template<typename T>
class ObjPool
{
	//保存大块内存的节点
	struct Node
	{
		Node* _mem;//指向大块内存的指针
		size_t _n; //当前节点里面的对象个数？
		Node* _next;//指向下一块的内存块

		Node(size_t nobjs)
		{
			_n = nobjs;
			_mem = (Node*)::operator new(_n*GetObjSize());//不能调用构造函数，否则会导致无限递归
			_next = NULL;
		}

		~Node()
		{
			_n = 0;
			::operator delete(_mem);//同上
			_mem = NULL;
			_next = NULL;
		}
	};
public:
	ObjPool(size_t initNobjs = 16, size_t maxNobjs = 1024)//构造函数
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
		//优先使用最近释放的内存块
		if (_lastUse)
		{
			void* obj = _lastUse;
			_lastUse = *(T**)_lastUse;
			printf("使用最近释放的内存块:%p\n", obj);
			return obj;
		}

		//表示没有内存可用了
		if (_useCount >= _tail->_n)
		{
			AllocNewNode();
			printf("没有可用内存,去申请\n");
		}

		//表示有可用的内存
		void* obj = (char*)_tail->_mem + _useCount*GetObjSize();
		_useCount++;
		printf("有可用的内存块:%p\n", obj);
		return obj;
	}

	void Deallocate(void* ptr)
	{
		if (ptr)
		{
			printf("释放内存块:%p\n", ptr);
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
	void AllocNewNode()//从系统中申请大块内存节点
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
	size_t _initObj;//想要申请最大内存块的个数
	size_t _maxObj;//最多可以申请内存块的个数
	Node* _head;//指向内存池的头节点
	Node* _tail;//指向内存池的尾节点
	size_t _useCount;//当前正在使用的节点个数
	T* _lastUse;//最近释放的内存块
};

void TestObjPool()
{
	ObjPool<string> pool;             //定义内存池对象
	string* p1 = (string*)pool.Allocate();//使用一个对象
	string* p2 = (string*)pool.Allocate();//使用一个对象
	pool.Deallocate(p1);                 //释放对象
	pool.Deallocate(p2);                 //释放对象
	string* p3 = (string*)pool.Allocate();//使用最近释放的对象
	string* p4 = (string*)pool.Allocate();//使用最近释放的对象
}