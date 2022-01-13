#include <memory>
#include <iostream>
#include <stack>
#include <queue>

template<typename T>
struct Node;

template<typename T>
using sNode = std::shared_ptr<Node<T>>;

template<typename T>
using wNode = std::weak_ptr<Node<T>>;


template<typename T>
struct Tree;

template<typename T>
using pTree = Tree<T>*;

template<typename T>
struct Node
{
	Node(T value) : _value{ value }
	{}

	T _value;
	sNode<T> _pLeft;
	sNode<T> _pRight;
};


template <typename U>
class Iterator {
public:
	Iterator(pTree<U> pData) : _pCurrent{ pData->_pHead } {}

	virtual void Begin() const = 0;
	virtual void Next() = 0;
	virtual bool IsDone() const = 0;

	sNode<U> Item() const {
		return this->_pCurrent;
	}

	virtual ~Iterator() = default;

protected:
	mutable sNode<U> _pCurrent;
};

template <typename U>
class DfsIterator : public Iterator<U> {

public:
	DfsIterator(Tree<U>* pData) : Iterator<U>(pData) { }

	void Begin() const override{
		Init(this->_pCurrent);
		this->_pCurrent = m_stack.top();
	}

	void Next() override {
		m_stack.pop();
		if (m_stack.empty())
			return;

		this->_pCurrent = m_stack.top();
	}


	bool IsDone() const override { return m_stack.empty(); }

private:
	void Init(sNode<U> pCurr) const
	{
		m_stack.push(pCurr);

		if (pCurr->_pRight)
			Init(pCurr->_pRight);

		if (pCurr->_pLeft)
			Init(pCurr->_pLeft);
	}

	mutable std::stack<sNode<U>> m_stack;
};


template <typename U>
class BfsIterator : public Iterator<U> {

public:
	BfsIterator(pTree<U> pData) : Iterator<U>(pData) { }

	void Begin() const override {
		Init(this->_pCurrent);
	}

	void Next() override {
		m_queue.pop();
		if (m_queue.empty())
			return;

		this->_pCurrent = m_queue.front();
	}

	bool IsDone() const override { return m_queue.empty(); }

private:
	void Init(sNode<U> pCurr) const
	{
		m_queue.push(pCurr);

		if (pCurr->_pLeft)
			Init(pCurr->_pLeft);

		if (pCurr->_pRight)
			Init(pCurr->_pRight);
	}

	mutable std::queue<sNode<U>> m_queue;

};

template<typename T>
struct Tree {
	sNode<T> _pHead;

	std::unique_ptr<Iterator<T>> NewDfsIterator()
	{
		return std::make_unique<DfsIterator<T>>(this);
	}

	std::unique_ptr<Iterator<T>> NewBfsIterator()
	{
		return std::make_unique<BfsIterator<T>>(this);
	}
};


int main()
{
	Tree<int> cutomTree;

	cutomTree._pHead = std::make_shared<Node<int>>(0);

	cutomTree._pHead->_pLeft = std::make_shared<Node<int>>(10);
	cutomTree._pHead->_pRight = std::make_shared<Node<int>>(20);


	cutomTree._pHead->_pLeft->_pLeft = std::make_shared<Node<int>>(100);
	cutomTree._pHead->_pLeft->_pRight = std::make_shared<Node<int>>(200);

	cutomTree._pHead->_pRight->_pLeft = std::make_shared<Node<int>>(300);
	cutomTree._pHead->_pRight->_pRight = std::make_shared<Node<int>>(400);
	


	auto dfs = cutomTree.NewDfsIterator();

	std::cout << "Dfs algo:\n";

	for (dfs->Begin(); !dfs->IsDone(); 	dfs->Next())
	{
		std::cout << dfs->Item()->_value << std::endl;
	}

	std::cout << "\nBfs algo:\n";

	auto bfs = cutomTree.NewBfsIterator();

	for (bfs->Begin(); !bfs->IsDone(); bfs->Next())
	{
		std::cout << bfs->Item()->_value << std::endl;
	}
}
