//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_ref.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::AVLTree is a binary search tree. Every node has a key and a value
// call ut::AVLTree::Insert() to add element, ut::AVLTree::Remove() to remove
// element and ut::AVLTree::Find() to get element by key. @Key type must
// have comparison operators implemented.
template <typename Key, typename Value>
class AVLTree
{
public:
	// L-Value reference types must be defined to provide ut::UniquePtr<>
	// compatibility for cpp dialects lower then C++11.
	typedef typename LValRef<Key>::Type KeyLRef;
	typedef typename LValRef<Value>::Type ValueLRef;

	// Each node of the tree has a key/value pair,
	// balance variable ('left side height' minus 'right side height')
	// and pointers to all adjacent nodes.
	class Node
	{
		friend AVLTree<Key, Value>;
	public:
		// value is the only public member
		Value value;

		// Constructor, key and value are copied
		Node(KeyLRef k,
			 ValueLRef v,
		     Node* p) : key(k)
		              , value(v)
		              , balance(0)
		              , parent(p)
		              , left(nullptr)
		              , right(nullptr)
		{}

#if CPP_STANDARD >= 2011
		// Constructor, key is copied, value is moved
		Node(KeyLRef k,
			 Value && v,
			 Node* p) : key(k)
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}

		// Constructor, key is moved, value is copied
		Node(Key && k,
			 ValueLRef v,
			 Node* p) : key(Move(k))
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}

		// Constructor, key and value are moved
		Node(Key && k,
			 Value && v,
			 Node* p) : key(Move(k))
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}
#endif // CPP_STANDARD >= 2011

		// Destructor
		~Node()
		{
			delete left;
			delete right;
		}

		// Returns a copy of the key member
		Key GetKey() const
		{
			return key;
		}

	private:
		// Returns constant pointer to the most left leaf node
		const Node* GetDeepestLeftChild() const
		{
			return left != nullptr ? left->GetDeepestLeftChild() : this;
		}

		// Returns constant pointer to the most right leaf node
		const Node* GetDeepestRightChild() const
		{
			return right != nullptr ? right->GetDeepestRightChild() : this;
		}

		// Returns pointer to the most left leaf node
		Node* GetDeepestLeftChild()
		{
			return left != nullptr ? left->GetDeepestLeftChild() : this;
		}

		// Returns pointer to the most right leaf node
		Node* GetDeepestRightChild()
		{
			return right != nullptr ? right->GetDeepestRightChild() : this;
		}

		// Returns pointer to the first parent that points to the current chain from the left side
		Optional<Node*> GetFirstLeftParent() const
		{
			if (parent != nullptr)
			{
				return parent->left == this ? parent : parent->GetFirstLeftParent();
			}
			return Optional<Node*>();
		}

		// Returns pointer to the first parent that points to the current chain from the right side
		Optional<Node*> GetFirstRightParent() const
		{
			if (parent != nullptr)
			{
				return parent->right == this ? parent : parent->GetFirstRightParent();
			}
			return Optional<Node*>();
		}

		// Returns the next sibling node in the parent tree
		//    @return - pointer to the sibling node or error if failed
		Optional<Node*> GetNextSibling() const
		{
			if (right != nullptr)
			{
				return right->GetDeepestLeftChild();
			}
			else if (parent != nullptr)
			{
				if (parent->left == this)
				{
					return parent;
				}
				else if (parent->right == this)
				{
					return parent->GetFirstLeftParent();
				}
			}

			return Optional<Node*>();
		}

		// Returns the previous sibling node in the parent tree
		//    @return - pointer to the sibling node or error if failed
		Optional<Node*> GetPreviousSibling() const
		{
			if (left != nullptr)
			{
				return left->GetDeepestRightChild();
			}
			else if (parent != nullptr)
			{
				if (parent->right == this)
				{
					return parent;
				}
				else if (parent->left == this)
				{
					return parent->GetFirstRightParent();
				}
			}

			return Optional<Node*>();
		}
		
		// key of any type that supports comparison operators
		Key key;

		// balance of the node
		int8 balance;

		// parent
		Node* parent;

		// childs
		Node* left;
		Node* right;
	};

private:
	// ut::AVLTree<>::IteratorTemplate is a template class for all avl tree iterators.
	// ut::AVLTree<>::Iterator is implemented as IteratorTemplate<Node> and
	// ut::AVLTree<>::ConstIterator is derived from IteratorTemplate<const Node>
	// to implement conversion from non-const (ut::AVLTree<>::ConstIterator)
	// iterator type.
	template<typename IteratorNodeType>
	class IteratorTemplate
	{
		friend AVLTree<Key, Value>;
		friend Node;
	public:
		// Default constructor
		IteratorTemplate() : node(nullptr)
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		IteratorTemplate(IteratorNodeType* ptr) : node(ptr)
		{ }

		// Copy constructor
		IteratorTemplate(const IteratorTemplate& copy) : node(copy.node)
		{ }

		// Assignment operator
		IteratorTemplate& operator = (const IteratorTemplate& copy)
		{
			node = copy.node;
			return *this;
		}

		// Returns constant reference of the managed object
		IteratorNodeType& operator*() const
		{
			return (*node);
		}

		// Inheritance operator, provides access to the owned object.
		// Return value can't be changed, it must be constant.
		IteratorNodeType* operator->() const
		{
			return node;
		}

		// Increment operator
		IteratorTemplate& operator++()
		{
			// node value 'nullptr' means the end of a tree
			if (node == nullptr)
			{
				return *this;
			}

			// move right
			Optional<Node*> sibling = node->GetNextSibling();
			if (sibling)
			{
				node = sibling.Get();
			}
			else
			{
				node = nullptr;
			}

			return *this;
		}

		// Post increment operator
		IteratorTemplate operator++(int)
		{
			IteratorTemplate tmp = *this;
			++(*this);
			return tmp;
		}

		// Decrement operator
		IteratorTemplate& operator--()
		{
			// node value 'nullptr' means the end of a tree
			if (node == nullptr)
			{
				return *this;
			}

			// move left
			Optional<Node*> sibling = node->GetPreviousSibling();
			if (sibling)
			{
				node = sibling.Get();
			}
			else
			{
				node = nullptr;
			}

			return *this;
		}

		// Post decrement operator
		IteratorTemplate operator--(int)
		{
			IteratorTemplate tmp = *this;
			node--;
			return tmp;
		}

		// Comparison operator 'equal to'
		bool operator == (const IteratorTemplate& right) const
		{
			return node == right.node;
		}

		// Comparison operator 'not equal to'
		bool operator != (const IteratorTemplate& right) const
		{
			return node != right.node;
		}

	protected:
		// managed object
		IteratorNodeType* node;
	};
	
public:
	// ut::AVLTree<>::Iterator is a bidirectional iterator
	// to iterate over the whole tree container.
	typedef IteratorTemplate<Node> Iterator;

	// ut::AVLTree<>::ConstIterator is a bidirectional constant iterator
	// to iterate over the whole tree container.
	class ConstIterator : public IteratorTemplate<const Node>
	{
		// Base iterator type
		typedef IteratorTemplate<const Node> Base;
	public:
		// Default constructor
		ConstIterator() : Base(nullptr)
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		ConstIterator(const Node* ptr) : Base(ptr)
		{ }

		// Copy constructor
		ConstIterator(const ConstIterator& copy) : Base(copy)
		{ }

		// Copy constructor
		ConstIterator(const Iterator& copy) : Base(copy)
		{ }

		// Assignment operator
		ConstIterator& operator = (const ConstIterator& copy)
		{
			Base::node = copy.node;
			return *this;
		}

		// Assignment operator (for non-const argument)
		ConstIterator& operator = (const Iterator& copy)
		{
			Base::node = &(*copy.node);
			return *this;
		}

		// Comparison operator 'equal to' for const argument
		bool operator == (const ConstIterator& right) const
		{
			return Base::node == &(*right.node);
		}

		// Comparison operator 'not equal to' for const argument
		bool operator != (const ConstIterator& right) const
		{
			return Base::node != &(*right.node);
		}

		// Comparison operator 'equal to' for non-const argument
		bool operator == (const Iterator& right) const
		{
			return Base::node == &(*right.node);
		}

		// Comparison operator 'not equal to' for non-const argument
		bool operator != (const Iterator& right) const
		{
			return Base::node != &(*right.node);
		}
	};

	// Constructor
	AVLTree() : root(nullptr)
	{}

	// Destructor, deletes all nodes
	~AVLTree()
	{
		delete root;
	}

	// Searches for the provided key
	Optional< Ref<Value> > Find(const Key& key)
	{
		// error if tree is empty
		if (root == nullptr)
		{
			return Optional< Ref<Value> >();
		}

		// check if root matches the key, otherwise - start to iterate child nodes
		if (root->key == key)
		{
			return Ref<Value>(root->value);
		}
		else
		{
			// intermediate variables for the loop
			Node* n = key > root->key ? root->right : root->left;

			// iterate nodes
			while (true)
			{
				// if we finished at nullptr leaf - there is no such key
				if (n == nullptr)
				{
					return Optional< Ref<Value> >();
				}
				else
				{
					if (n->key == key)
					{
						return Ref<Value>(n->value);
					}
					else
					{
						n = key > n->key ? n->right : n->left;
					}
				}
			}
		}
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(KeyLRef key, ValueLRef value)
	{
#if CPP_STANDARD >= 2011
		return EmplacePair(key, value);
#else
		// MS Visual Studio can't deduce template arguments
		// if they were defined via proxy ut::LValRef type
		return EmplacePair<Key, Value>(key, value);
#endif
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(Key && key, Value && value)
	{
		return EmplacePair(Move(key), Move(value));
	}
#endif

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(KeyLRef key, Value && value)
	{
		return EmplacePair(key, Move(value));
	}
#endif

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(Key && key, ValueLRef value)
	{
		return EmplacePair(Move(key), value);
	}
#endif

	// Removes node from the tree
	//    @param key - key of the node to be deleted
	void Remove(const Key& key)
	{
		// exit if tree is empty
		if (root == nullptr)
		{
			return;
		}

		// intermediate variables for the 'while' loop
		Node* n = root, *parent = root, *node_to_delete = nullptr, *child = root;

		// search for the node to delete
		while (child != nullptr)
		{
			// set a node from the previous cycle as a parent node
			parent = n;

			// new node is a child of the previous one
			n = child;

			// get a child of the node for the next cycle
			child = key >= n->key ? n->right : n->left;

			// if keys match - we found the node to delete
			if (key == n->key)
			{
				node_to_delete = n;
			}
		}

		// if such node was found - we should rebalance the tree
		if (node_to_delete != nullptr)
		{
			// set key
			node_to_delete->key = n->key;

			// set child
			child = n->left != nullptr ? n->left : n->right;

			// check if desired node is a root node
			if (root->key == key)
			{
				// process case when the root is a node to be removed
				root = child;
			}
			else
			{
				// link parent with child
				if (parent->left == n)
				{
					parent->left = child;
				}
				else
				{
					parent->right = child;
				}
				
				// rebalance the tree
				Rebalance(parent);
			}

			// remove all links from the node
			n->left = nullptr;
			n->right = nullptr;

			// delete unreferenced node
			delete n;
		}
	}

	// Returns constant read / write iterator that points to the first element
	ConstIterator Begin(iterator::Position position = iterator::first) const
	{
		if (root != nullptr)
		{
			return ConstIterator(position == iterator::first ? root->GetDeepestLeftChild() : root->GetDeepestRightChild());
		}
		else
		{
			return ConstIterator(nullptr);
		}
	}

	// Returns constant read / write iterator that points to the last element
	ConstIterator End(iterator::Position position = iterator::last) const
	{
		return ConstIterator(nullptr);
	}

	// Returns constant read / write iterator that points to the first element
	Iterator Begin(iterator::Position position = iterator::first)
	{
		if (root != nullptr)
		{
			return Iterator(position == iterator::first ? root->GetDeepestLeftChild() : root->GetDeepestRightChild());
		}
		else
		{
			return Iterator(nullptr);
		}
	}

	// Returns constant read / write iterator that points to the last element
	Iterator End(iterator::Position position = iterator::last)
	{
		return Iterator(nullptr);
	}

private:
	// Inserts new key-value pair to the tree
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	template <typename KeyType, typename ValueType>
#if CPP_STANDARD >= 2011
	inline bool EmplacePair(KeyType && key, ValueType && value)
#else
	inline bool EmplacePair(KeyLRef key, ValueLRef value)
#endif
	{
		// check if root exists
		if (root == nullptr)
		{
			// create new node for the root 
			// if this is the first insertion
			root = new Node(Forward<KeyType>(key), Forward<ValueType>(value), nullptr);
		}
		else
		{
			// intermediate variables for the loop
			Node* n = root, *parent;

			// iterate nodes till appropriate place for the new key is not found
			while (true)
			{
				// prevent insertion of the non-unique key
				if (n->key == key)
				{
					return false;
				}

				// set a node from the previous cycle as a parent node
				parent = n;

				// if key is less than node @n then we go the left node,
				// otherwise - to the right one
				bool left_side = n->key > key;
				n = left_side ? n->left : n->right;

				// if node @n exists - proceed to the next loop iteration untill reaching empty node
				if (n == nullptr)
				{
					// create a new child for the node @n
					if (left_side)
					{
						parent->left = new Node(Forward<KeyType>(key), Forward<ValueType>(value), parent);
					}
					else
					{
						parent->right = new Node(Forward<KeyType>(key), Forward<ValueType>(value), parent);
					}

					// rebalance tree chunk starting from the @parent node
					Rebalance(parent);

					// break the loop
					break;
				}
			}
		}

		// node was successfully inserted
		return true;
	}

	// 'Left Left' rotation
	//    @parameter a - parent node
	//    @return - new parent
	Node* RotateLeft(Node* a)
	{
		// get new parent node
		Node* b = a->right;

		// link child and parent
		b->parent = a->parent;
		a->right = b->left;

		// set new parent for the right child of the @a node
		if (a->right != nullptr)
		{
			a->right->parent = a;
		}

		// swap @a and @b
		b->left = a;
		a->parent = b;

		// link the new child for the another child of the parent
		if (b->parent != nullptr)
		{
			if (b->parent->right == a)
			{
				b->parent->right = b;
			}
			else
			{
				b->parent->left = b;
			}
		}

		// update balances
		UpdateBalance(a);
		UpdateBalance(b);

		// return new parent node
		return b;
	}

	// 'Right Right' rotation
	//    @parameter a - parent node
	//    @return - new parent
	Node* RotateRight(Node* a)
	{
		// get new parent node
		Node* b = a->left;

		// link child and parent
		b->parent = a->parent;
		a->left = b->right;

		// set new parent for the left child of the @a node
		if (a->left != nullptr)
		{
			a->left->parent = a;
		}

		// swap @a and @b
		b->right = a;
		a->parent = b;

		// link the new child for the another child of the parent
		if (b->parent != nullptr)
		{
			if (b->parent->right == a)
			{
				b->parent->right = b;
			}
			else
			{
				b->parent->left = b;
			}
		}

		// update balances
		UpdateBalance(a);
		UpdateBalance(b);

		// return new parent node
		return b;
	}

	// 'Left Right' rotation
	//    @parameter n - parent node
	//    @return - new parent
	Node* RotateLeftThenRight(Node* n)
	{
		n->left = RotateLeft(n->left);
		return RotateRight(n);
	}

	// 'Right Left' rotation
	//    @parameter n - parent node
	//    @return - new parent
	Node* RotateRightThenLeft(Node* n)
	{
		n->right = RotateRight(n->right);
		return RotateLeft(n);
	}

	// Rebalances the tree starting from the specified node
	//    @param n - lowest node to rebalance from
	void Rebalance(Node* n)
	{
		// update balance for the current node
		UpdateBalance(n);

		// choose appropriate rotation according to the balance value
		if (n->balance == -2)
		{
			if (GetHeight(n->left->left) >= GetHeight(n->left->right))
			{
				n = RotateRight(n);
			}
			else
			{
				n = RotateLeftThenRight(n);
			}
		}
		else if (n->balance == 2)
		{
			if (GetHeight(n->right->right) >= GetHeight(n->right->left))
			{
				n = RotateLeft(n);
			}
			else
			{
				n = RotateRightThenLeft(n);
			}
		}

		// we are going higher recursively
		if (n->parent != nullptr)
		{
			Rebalance(n->parent);
		}
		else
		{
			root = n;
		}
	}

	// Return the maximum height of the sub-tree
	//    @param n - first node to calculate height for
	//    @return - height of the sub-tree
	int GetHeight(Node* n)
	{
		// if node is empty, balance is -1
		// (left side is calculated first)
		if (n == nullptr)
		{
			return -1;
		}

		// calculate height for both sides
		int l_heght = GetHeight(n->left);
		int r_heght = GetHeight(n->right);

		// pick the bigger side
		int mh = l_heght > r_heght ? l_heght : r_heght;

		// return the result
		return 1 + mh;
	}

	// Updates the balance for the desired node
	//    @param n - node to be rebalanced
	void UpdateBalance(Node* n)
	{
		n->balance = GetHeight(n->right) - GetHeight(n->left);
	}

	// root node, null by default
	Node* root;

};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//