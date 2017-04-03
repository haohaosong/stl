#pragma once 

//引入空间配置器
#include"Allocator.h"

//定义链表的节点
template<typename T>
struct MyListNode
{
	T _data;
	MyListNode<T>* _pNext;
	MyListNode<T>* _pPre;

	MyListNode(T data)
		:_data(data)
		, _pPre(NULL)
		, _pNext(NULL)
	{}
};

template<typename T,typename Ref,typename Ptr>
struct MyListIterator
{
	typedef MyListNode<T> Node;
	typedef MyListIterator<T,Ref,Ptr> Self;

	Self& operator++()
	{
		_node = _node->_pNext;
		return *this;
	}

	Self operator++(int)
	{
		Node* old = _node;
		_node = _node->_pNext;
		return old;
	}

	Self& operator--()
	{
		_node = _node->_pPre;
		return *this;
	}

	Self operator--(int)
	{
		Node* old = _node;
		_node = _node->_pPre;
		return old;
	}

	bool operator==(const Self &s)
	{
		return _node == s._node;
	}

	bool operator!=(const Self &s)
	{
		return _node != s._node;
	}

	inline Ref operator*()const 
	{
		return _node->_data;
	}

	inline Ptr operator->()const
	{
		return _node->_data;
	}

	MyListIterator(Node* node)
		:_node(node)
	{}

	Node* _node;
};

//定义带头的链表
template<typename T,typename Alloc = _alloc>
class MyList
{
	typedef MyListNode<T> Node;
	typedef SimpleAlloc<Node, Alloc> DataAllocator;
public:
	typedef MyListIterator<T, T&, T*> Iterator;
	typedef MyListIterator<T, const T&, const T*> ConstIterator;
public:
	Iterator Begin()
	{
		return _head->_pNext;
	}

	Iterator End()
	{
		return _head;
	}

	ConstIterator Begin()const
	{
		return _head->_pNext;
	}

	ConstIterator End()const
	{
		return _head;
	}
public:
	//构造函数，带头节点的双向链表
	MyList()
	{
		_head = _BuyNode(T());
		_head->_pNext = _head;
		_head->_pPre = _head;
	}

	//压入一个数据
	void PushBack(T data)
	{
		Node* newNode = _BuyNode(data);
		Node* lastNode = _head->_pPre;

		lastNode->_pNext = newNode;
		_head->_pPre = newNode;
		newNode->_pPre = lastNode;
		newNode->_pNext = _head;
	}

	//清空链表，使该链表可以被用作其他用途
	void Clear()
	{
		Node* cur = _head->_pNext;
		while (cur != _head)
		{
			Node* delNode = cur;
			cur = cur->_pNext;
			_DestoryNode(cur);
		}
	}

	~MyList()
	{
		Clear();

		//别忘记释放头节点
		_DestoryNode(_head);
		_head = NULL;
	}
protected:
	//没有引入空间配置器
	//Node* _BuyNode(T data)
	//{
	//	Node* newNode = new Node(data);
	//	return newNode;
	//}

	//void _DestoryNode(Node* p)
	//{
	//	//显示的调用析够函数，释放Node的空间
	//	p->~Node();
	//}

	//引入空间配置器后，仅修改这两个函数
	Node* _BuyNode(T data)
	{
		Node* newNode = DataAllocator::Allocate();
		new(newNode)Node(data);
		return newNode;
	}

	void _DestoryNode(Node* p)
	{
		DataAllocator::Deallocate(p);
	}
protected:
	Node* _head;
};

void TestMyList()
{
	MyList<int> l;
	l.PushBack(1);
	l.PushBack(2);
	l.PushBack(3);
	l.PushBack(4);

	MyList<int>::Iterator it = l.Begin();
	while (it != l.End())
	{
		cout << *it << " ";
		++it;
	}

	cout << endl;

	/*const MyList<int> cl;
	MyList<int>::ConstIterator it1 = cl.Begin();
	MyList<int>::ConstIterator it2 = cl.End();
	cout << (it1 == it2) << endl;*/
}