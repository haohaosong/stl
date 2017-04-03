#pragma once 

//����ռ�������
#include"Allocator.h"

//��������Ľڵ�
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

//�����ͷ������
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
	//���캯������ͷ�ڵ��˫������
	MyList()
	{
		_head = _BuyNode(T());
		_head->_pNext = _head;
		_head->_pPre = _head;
	}

	//ѹ��һ������
	void PushBack(T data)
	{
		Node* newNode = _BuyNode(data);
		Node* lastNode = _head->_pPre;

		lastNode->_pNext = newNode;
		_head->_pPre = newNode;
		newNode->_pPre = lastNode;
		newNode->_pNext = _head;
	}

	//�������ʹ��������Ա�����������;
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

		//�������ͷ�ͷ�ڵ�
		_DestoryNode(_head);
		_head = NULL;
	}
protected:
	//û������ռ�������
	//Node* _BuyNode(T data)
	//{
	//	Node* newNode = new Node(data);
	//	return newNode;
	//}

	//void _DestoryNode(Node* p)
	//{
	//	//��ʾ�ĵ��������������ͷ�Node�Ŀռ�
	//	p->~Node();
	//}

	//����ռ��������󣬽��޸�����������
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