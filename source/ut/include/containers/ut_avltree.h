//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_allocator.h"
#include "containers/ut_ref.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Forward declaration of ut::meta::Parameter template class.
namespace meta { template<typename> class Parameter; }

//----------------------------------------------------------------------------//
// ut::AVLTree is a binary search tree. Every node has a key and a value
// call ut::AVLTree::Insert() to add element, ut::AVLTree::Remove() to remove
// element and ut::AVLTree::Find() to get element by key. @Key type must
// have comparison operators implemented.
template <typename Key, typename Value, template<typename> class Allocator = DefaultAllocator>
class AVLTree
{
	// ut::meta::Parameter must be a friend so that ut::AVLTree could be serializable.
	template <typename> friend class meta::Parameter;
public:

	// Each node of the tree has a key/value pair,
	// balance variable ('left side height' minus 'right side height')
	// and pointers to all adjacent nodes.
	class Node
	{
		// ut::meta::Parameter must be a friend so that
		// ut::AVLTree::Node could be serializable.
		template <typename> friend class meta::Parameter;

		// ut::AVLTree must be a friend in order to be able to operate with nodes.
		template<typename, typename, template<typename> class> friend class AVLTree;
	public:
		// value is the only public member
		Value value;

		// Constructor, key and value are copied
		Node(const Key& k,
		     const Value& v,
		     Node* p) : key(k)
		              , value(v)
		              , balance(0)
		              , parent(p)
		              , left(nullptr)
		              , right(nullptr)
		{}

		// Constructor, key is copied, value is moved
		Node(const Key& k,
			 Value&& v,
			 Node* p) : key(k)
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}

		// Constructor, key is moved, value is copied
		Node(Key&& k,
			 const Value& v,
			 Node* p) : key(Move(k))
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}

		// Constructor, key and value are moved
		Node(Key&& k,
			 Value&& v,
			 Node* p) : key(Move(k))
			          , value(Move(v))
			          , balance(0)
			          , parent(p)
			          , left(nullptr)
			          , right(nullptr)
		{}

		// Recursively destroys child nodes and deallocates memory.
		void DestroyLeaves(Allocator<Node>& allocator)
		{
			if (left != nullptr)
			{
				left->DestroyLeaves(allocator);
				left->~Node();
				allocator.Deallocate(left, 1);
			}

			if (right != nullptr)
			{
				right->DestroyLeaves(allocator);
				right->~Node();
				allocator.Deallocate(right, 1);
			}
		}

		// Returns const reference to the key
		const Key& GetKey() const
		{
			return key;
		}

		// Returns current balance value
		int8 GetBalance() const
		{
			return balance;
		}

		// Returns pointer to the left leaf
		const Node* GetLeft() const
		{
			return left;
		}

		// Returns pointer to the right leaf
		const Node* GetRight() const
		{
			return right;
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

	// Copy constructor
	AVLTree(const AVLTree& copy) : root(nullptr)
	                             , allocator(copy.allocator)
	{
		CopyNode(root, copy.root, nullptr);
	}

	// Move constructor
	AVLTree(AVLTree&& right) noexcept : root(right.root)
	                                  , allocator(right.allocator)
	{
		right.root = nullptr;
	}

	// Assignment operator
	AVLTree& operator = (const AVLTree& copy)
	{
		DeleteNode(root);
		allocator = copy.allocator;
		CopyNode(root, copy.root, nullptr);
		return *this;
	}

	// Move operator
	AVLTree& operator = (AVLTree&& right) noexcept
	{
		DeleteNode(root);
		allocator = right.allocator;
		root = right.root;
		right.root = nullptr;
		return *this;
	}

	// Destructor, deletes all nodes
	~AVLTree()
	{
		DeleteNode(root);
	}

	// Searches for a value by key.
	//    @param key - desired key
	//    @return - reference to the value if found
	Optional<Value&> Find(const Key& key)
	{
		return FindValue<Value&>(key);
	}

	// Searches for a value by key, value can't be changed.
	//    @param key - desired key
	//    @return - const reference to the value if found
	Optional<const Value&> Find(const Key& key) const
	{
		return FindValue<const Value&>(key);
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(const Key& key, const Value& value)
	{
		return EmplacePair(key, value);
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(Key&& key, Value&& value)
	{
		return EmplacePair(Move(key), Move(value));
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(const Key& key, Value&& value)
	{
		return EmplacePair(key, Move(value));
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(Key&& key, const Value& value)
	{
		return EmplacePair(Move(key), value);
	}

	// Removes node from the tree
	//    @param key - key of the node to be deleted
	void Remove(const Key& key)
	{
		root = DeleteNode(root, key);
	}

	// Destructs all nodes
	void Reset()
	{
		DeleteNode(root);
		root = nullptr;
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

	// Returns a reference to the allocator
	Allocator<Node>& GetAllocator()
	{
		return allocator;
	}

	// Returnsconst  reference to the allocator
	const Allocator<Node>& GetAllocator() const
	{
		return allocator;
	}

private:
	// Copies provided node with all it's leaves.
	//    @param node - reference to the destination node pointer
	//    @param copy - const pointer to the node to be copied
	//    @param parent - pointer to the parent node to be linked with
	void CopyNode(Node*& node, const Node* copy, Node* parent)
	{
		// skip if copy doesn't exist
		if (copy == nullptr)
		{
			node = nullptr;
			return;
		}

		// create a copy
		node = allocator.Allocate(1);
		new(node) Node(copy->key, copy->value, parent);

		// set balance - it stays immutable after copying
		node->balance = copy->balance;

		// do the same as above with both leaves (left and right)
		CopyNode(node->left, copy->left, node);
		CopyNode(node->right, copy->right, node);
	}

	// Inserts new key-value pair to the tree
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	template <typename KeyType, typename ValueType>
	inline bool EmplacePair(KeyType && key, ValueType && value)
	{
		// check if root exists
		if (root == nullptr)
		{
			// create new node for the root 
			// if this is the first insertion
			root = allocator.Allocate(1);
			new(root) Node(Forward<KeyType>(key), Forward<ValueType>(value), nullptr);
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
				const bool left_side = n->key > key;
				n = left_side ? n->left : n->right;

				// if node @n exists - proceed to the next loop iteration untill reaching empty node
				if (n == nullptr)
				{
					// create a new child for the node @n
					if (left_side)
					{
						parent->left = allocator.Allocate(1);
						new(parent->left) Node(Forward<KeyType>(key), Forward<ValueType>(value), parent);
					}
					else
					{
						parent->right = allocator.Allocate(1);
						new(parent->right) Node(Forward<KeyType>(key), Forward<ValueType>(value), parent);
					}

					// rebalance tree chunk starting from the @parent node
					RebalanceTree(parent);

					// break the loop
					break;
				}
			}
		}

		// node was successfully inserted
		return true;
	}

	// Searches for a value by key.
	// @ReturnType can be either ut::AVLTree::Value& or const ut::AVLTree::Value&.
	//    @param key - desired key
	//    @return - @ReturnType if key was found
	template<typename ReturnType>
	Optional<ReturnType> FindValue(const Key& key) const
	{
		// error if tree is empty
		if (root == nullptr)
		{
			return Optional<ReturnType>();
		}

		// check if root matches the key, otherwise - start to iterate child nodes
		if (root->key == key)
		{
			return root->value;
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
					return Optional<ReturnType>();
				}
				else
				{
					if (n->key == key)
					{
						return n->value;
					}
					else
					{
						n = key > n->key ? n->right : n->left;
					}
				}
			}
		}
	}

	// Returns the node with minimum key value found in that tree.
	// Note that the entire tree does not need to be searched.
	//    @param node - pointer to the root of the tree to search in
	//    @return - pointer to the node with minimum key value
	Node* GetMinValueNode(Node* node)
	{
		// loop down to find the leftmost leaf
		Node* current = node;
		while (current->left != nullptr)
		{
			current = current->left;
		}
		return current;
	}

	// Destroys desired node and deallocates memory.
	void DeleteNode(Node* node)
	{
		if (node == nullptr)
		{
			return;
		}

		node->DestroyLeaves(allocator);
		node->~Node();
		allocator.Deallocate(node, 1);
	}

	// Recursive function to delete a node with given key from subtree with given root.
	// It returns root of the modified subtree.  
	//    @param parent - current root of subtree
	//    @param key - key of the node to be deleted
	//    @return - pointer to the  root of the modified subtree
	Node* DeleteNode(Node* parent, const Key& key)
	{
		// validate argument
		if (parent == nullptr)
		{
			return parent;
		}

		// if the key to be deleted is smaller than the
		// parent's key, then it lies in left subtree
		if (key < parent->key)
		{
			parent->left = DeleteNode(parent->left, key);
		}
		else if (key > parent->key) // If the key to be deleted is greater than the
		{                           // parent's key, then it lies in right subtree
			parent->right = DeleteNode(parent->right, key);
		}
		else // if key is same as parent's key, then this is the node to be deleted
		{
			// node with only one child or no child  
			if ((parent->left == nullptr) || (parent->right == nullptr))
			{
				Node *temp = parent->left ? parent->left : parent->right;

				// no child case  
				if (temp == nullptr)
				{
					temp = parent;
					parent = nullptr;
				}
				else // one child case
				{
					// copy the contents of the non-empty child
					parent->key = temp->key;
					parent->value = Move(temp->value);
					parent->balance = temp->balance;
					parent->left = temp->left;
					parent->right = temp->right;
				}

				// unlinked node can be deleted now
				DeleteNode(temp);
			}
			else
			{
				// node with two children: get the inorder  
				// successor (smallest in the right subtree)  
				Node* temp = GetMinValueNode(parent->right);

				// copy the inorder successor's data to this node  
				parent->key = temp->key;
				parent->value = Move(temp->value);

				// delete the inorder successor  
				parent->right = DeleteNode(parent->right, parent->key);
			}
		}

		// if the tree had only one node then return  
		if (parent == nullptr)
		{
			return parent;
		}

		// rebalance node
		return RebalanceNode(parent);
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

	// Rebalances desired node
	//    @param n - node to rebalance
	//    @return - root of the rebalanced tree
	Node* RebalanceNode(Node* n)
	{
		// update balance for the current node
		UpdateBalance(n);

		// choose appropriate rotation according to the balance value
		if (n->balance < -1)
		{
			if (GetHeight(n->left->left) >= GetHeight(n->left->right))
			{
				return RotateRight(n);
			}
			else
			{
				return RotateLeftThenRight(n);
			}
		}
		else if (n->balance > 1)
		{
			if (GetHeight(n->right->right) >= GetHeight(n->right->left))
			{
				return RotateLeft(n);
			}
			else
			{
				return RotateRightThenLeft(n);
			}
		}

		return n;
	}

	// Rebalances the tree starting from the specified node
	//    @param n - lowest node to rebalance from
	void RebalanceTree(Node* n)
	{
		// rebalance current node
		n = RebalanceNode(n);

		// we are going higher recursively
		if (n->parent != nullptr)
		{
			RebalanceTree(n->parent);
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

	// allocator
	Allocator<Node> allocator;
};

//----------------------------------------------------------------------------//
// Specialize type name function for avltree
template<typename Key, typename Value> struct Type< AVLTree<Key, Value> >
{
	static inline const char* Name() { return "avltree"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//