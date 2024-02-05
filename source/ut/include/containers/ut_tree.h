//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "templates/ut_ref.h"
#include "containers/ut_iterator.h"
#include "containers/ut_array.h"
#include "pointers/ut_unique_ptr.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::BaseTree is a base template class for unordered trees. You can expand
// it's functionality using curiously recurring template pattern in derived
// class (see ut::Tree<> template class for example), also you can specialize
// ut::Tree for your data typw (see how it's derived from ut::BaseTree in
// default template version).
template<typename T, typename TreeContainer>
class BaseTree
{
protected:
	// Definitions for node and data types.
	typedef TreeContainer NodeType;
	typedef T DataType;

	// ut::BaseTree<>::IteratorTemplate is a template class for all tree iterators.
	// ut::BaseTree<>::Iterator is implemented as IteratorTemplate<NodeType> and
	// ut::BaseTree<>::ConstIterator is derived from IteratorTemplate<const NodeType>
	// to implement conversion from non-const (ut::BaseTree<>::ConstIterator)
	// iterator type.
	template<typename IteratorNodeType>
	class IteratorTemplate
	{
		friend BaseTree<DataType, NodeType>;
		friend NodeType;
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

			// step down if node has at least one child
			if (node->CountChildren() > 0)
			{
				node = &node->child_nodes[0];
			}
			else
			{
				// step up or right otherwise
				Result<NodeType*, Error> result = node->GetNextSibling();
				if (result)
				{
					node = result.Get();
				}
				else
				{
					node = nullptr;
				}
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

			// step up if node has at least one child
			Result<NodeType*, Error> result = node->GetPreviousNode();
			if (result)
			{
				node = result.Get();
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
	// ut::BaseTree<>::Iterator is a bidirectional iterator
	// to iterate over the whole tree container. This class is capable only to read
	// the content of the container. Use ut::Array<>::Iterator if you want to
	// write (modify) the container data.
	typedef IteratorTemplate<NodeType> Iterator;

	// ut::BaseTree<>::ConstIterator is a bidirectional constant iterator
	// to iterate over the whole tree container. This class is capable only to read
	// the content of the container. Use ut::Array<>::Iterator if you want to
	// write (modify) the container data.
	class ConstIterator : public IteratorTemplate<const NodeType>
	{
		// Base iterator type
		typedef IteratorTemplate<const NodeType> Base;
	public:
		// Default constructor
		ConstIterator() : Base(nullptr)
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		ConstIterator(const NodeType* ptr) : Base(ptr)
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

	// Constructor, @data has default value
	BaseTree() : parent(nullptr), id(0)
	{}

	// Constructor, @data is copied from parameter
	BaseTree(const DataType& data_copy) : data(data_copy), parent(nullptr), id(0)
	{}

	// Constructor, @data is moved from parameter
	BaseTree(DataType&& rval_data) : data(Move(rval_data)), parent(nullptr), id(0)
	{}

	// Copy constructor
	BaseTree(const BaseTree& copy) : data(copy.data)
	                               , parent(copy.parent)
	                               , id(copy.id)
	                               , child_nodes(copy.child_nodes)
	{
		ResetChildsId();
	}

	// Move constructor
	BaseTree(BaseTree&& other) noexcept : data(Move(other.data))
	                                    , parent(other.parent)
	                                    , id(other.id)
	                                    , child_nodes(Move(other.child_nodes))
	{
		ResetChildsId();
	}

	// Assignment operator
	BaseTree& operator = (const BaseTree& copy)
	{
		data = copy.data;
		parent = copy.parent;
		id = copy.id;
		child_nodes = copy.child_nodes;
		ResetChildsId();
		return *this;
	}

	// Move operator
	BaseTree& operator = (BaseTree&& other) noexcept
	{
		data = Move(other.data);
		parent = other.parent;
		id = other.id;
		child_nodes = Move(other.child_nodes);
		ResetChildsId();
		return *this;
	}

	// Returns desired element from child array
	inline NodeType& operator [] (const size_t id)
	{
		return child_nodes[id];
	}

	// Returns desired element from child array
	inline const NodeType& operator [] (const size_t id) const
	{
		return child_nodes[id];
	}

	// Returns desired element from the whole tree
	NodeType& operator () (const size_t id)
	{
		Result<NodeType*, size_t> result = Iterate(id);
		return result ? *result.Get() : *static_cast<NodeType*>(this);
	}

	// Returns desired element from the whole tree
	const NodeType& operator () (const size_t id) const
	{
		Result<const NodeType*, size_t> result = Iterate(id);
		return result ? *result.Get() : *static_cast<NodeType*>(this);
	}

	// Adds new child node to the end of the array
	// @data_copy is a constant reference
	//    @param copy - new element
	bool Add(const NodeType& child_node)
	{
		if (!child_nodes.Add(child_node))
		{
			return false;
		}
		ResetChildsId();
		return true;
	}

	// Moves new child node to the end of the array
	// @data_copy is a constant reference
	//    @param child_node - r-value reference to a new element
	bool Add(NodeType&& child_node)
	{
		if (!child_nodes.Add(Move(child_node)))
		{
			return false;
		}
		ResetChildsId();
		return true;
	}

	// Adds new child node to the end of the array (reference)
	// @data_copy is a constant reference
	//    @param copy - new element
	bool Add(const DataType& data_copy)
	{
		return EmplaceBack(data_copy);
	}

	// Adds new child node to the end of the array (r-value reference)
	// uses move semantics to perform copying
	//    @param copy - new element
	bool Add(DataType&& data_rval)
	{
		return EmplaceBack(Move(data_rval));
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param copy - element to be copied
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(size_t position, const DataType& copy)
	{
		return Emplace(copy, position);
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param element - element to be moved
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(size_t position, DataType&& element)
	{
		return Emplace(Move(element), position);
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param copy - element to be copied
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(typename Array<NodeType>::ConstIterator position, const DataType& copy)
	{
		Optional<size_t> child_id = ConvertChildIdFromIterator(position);
		if (child_id && Emplace(copy, child_id.Get()))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param element - element to be moved
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(typename Array<NodeType>::ConstIterator position, DataType&& element)
	{
		Optional<size_t> child_id = ConvertChildIdFromIterator(position);
		if (child_id && Emplace(Move(element), child_id.Get()))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// Removes desired child element
	//    @param node_id - index of the child node
	void Remove(size_t node_id)
	{
		if (node_id < child_nodes.Count())
		{
			// remove child node
			child_nodes.Remove(node_id);

			// re-assign id of the child nodes
			ResetChildsId();
		}
	}

	// Removes desired child element
	//    @param iterator - element's position
	void Remove(typename Array<NodeType>::ConstIterator iterator)
	{
		Optional<size_t> child_id = ConvertChildIdFromIterator(iterator);
		if (child_id)
		{
			Remove(child_id.Get());
		}
	}

	// Removes child nodes of the tree
	void Reset()
	{
		child_nodes.Reset();
	}

	// Returns the parent node
	Optional<const NodeType&> GetParent() const
	{
		return parent != nullptr ? *parent : Optional<const NodeType&>();
	}

	// Returns the parent node
	Optional<NodeType&> GetParent()
	{
		return parent != nullptr ? *parent : Optional<NodeType&>();
	}

	// Returns the last one-level child node
	//    @return - constant reference to the last child node
	const NodeType& GetLastChild() const
	{
		return child_nodes.GetLast();
	}

	// Returns the last one-level child node
	//    @return - constant reference to the last child node
	NodeType& GetLastChild()
	{
		return child_nodes.GetLast();
	}

	// Returns the number of child nodes
	inline size_t CountChildren() const
	{
		return child_nodes.Count();
	}

	// Returns the number of all nodes
	inline size_t Count() const
	{
		size_t total_num = 1;
		for (size_t i = 0; i < child_nodes.Count(); i++)
		{
			total_num += child_nodes[i].Count();
		}
		return total_num;
	}

	// Returns constant read / write iterator that points to the first element
	typename Array<NodeType>::ConstIterator BeginLeaves(iterator::Position position = iterator::first) const
	{
		return child_nodes.Begin(position);
	}

	// Returns constant read / write iterator that points to the last element
	typename Array<NodeType>::ConstIterator EndLeaves(iterator::Position position = iterator::last) const
	{
		return child_nodes.End(position);
	}

	// Returns a read / write iterator that points to the first element
	typename Array<NodeType>::Iterator BeginLeaves(iterator::Position position = iterator::first)
	{
		return child_nodes.Begin(position);
	}

	// Returns a read / write iterator that points to the last element
	typename Array<NodeType>::Iterator EndLeaves(iterator::Position position = iterator::last)
	{
		return child_nodes.End(position);
	}

	// Returns constant read / write iterator that points to the first element
	ConstIterator Begin(iterator::Position position = iterator::first) const
	{
		return position == iterator::first ? ConstIterator(static_cast<NodeType*>(this)) : ConstIterator(GetLastNode());
	}

	// Returns constant read / write iterator that points to the last element
	ConstIterator End(iterator::Position position = iterator::last) const
	{
		return ConstIterator(nullptr);
	}

	// Returns constant read / write iterator that points to the first element
	Iterator Begin(iterator::Position position = iterator::first)
	{
		return position == iterator::first ? Iterator(static_cast<NodeType*>(this)) : Iterator(GetLastNode());
	}

	// Returns constant read / write iterator that points to the last element
	Iterator End(iterator::Position position = iterator::last)
	{
		return Iterator(nullptr);
	}

	// value of the node
	DataType data;

protected:
	// Inserts elements at the specified location in the container.
	//    @param copy - data to be copied
	//    @param position - iterator before which the content will be inserted
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	template <typename ArgType>
	inline bool Emplace(ArgType && copy, size_t position)
	{
		// insert new child node
		if (!child_nodes.Insert(position, Forward<ArgType>(copy)))
		{
			return false;
		}

		// set the parent of the new node
		child_nodes[position].parent = static_cast<NodeType*>(this);

		// reset all child nodes' id
		ResetChildsId();

		// success
		return true;
	}

	// Adds new child node to the end of the array
	//    @param copy - data to be copied
	//    @return - true if succeeded
	template <typename ArgType>
	inline bool EmplaceBack(ArgType && copy)
	{
		// add new node to the end of the array
		if (!child_nodes.Add(NodeType(Forward<ArgType>(copy))))
		{
			return false;
		}

		// set parent and id of the new node
		size_t child_id = child_nodes.Count() - 1;
		child_nodes[child_id].parent = static_cast<NodeType*>(this);
		child_nodes[child_id].id = child_id;

		// success
		return true;
	}

	// Iterates all nodes inside a tree searching for the desired node
	//    @param position - position of the desired node
	//    @return - pointer of the node, or new position if node was not found
	Result<NodeType*, size_t> Iterate(size_t position) const
	{
		NodeType* node = static_cast<NodeType*>(this);
		size_t child_id = 0;
		while (position)
		{
			if (child_id >= child_nodes.Count())
			{
				return MakeAlt<size_t>(position);
			}
			else
			{
				position--;
				Result<NodeType*, size_t> result = child_nodes[child_id].Iterate(position);
				if (result)
				{
					return result.Get();
				}
				else
				{
					child_id++;
					position = result.GetAlt();
				}
			}
		}
		return node;
	}

	// Returns the next sibling node in the parent tree
	//    @return - pointer to the sibling node or error if failed
	Result<NodeType*, Error> GetNextSibling() const
	{
		if (parent == nullptr)
		{
			return MakeError(error::out_of_bounds);
		}

		size_t sibling_id = id + 1;
		if (sibling_id < parent->child_nodes.Count())
		{
			return &parent->child_nodes[sibling_id];
		}
		else
		{
			return parent->GetNextSibling();
		}
	}

	// Returns the previous node in the tree (according to the current node's id)
	//    @return - pointer to the previous node or error if failed
	Result<NodeType*, Error> GetPreviousNode() const
	{
		if (parent == nullptr)
		{
			return MakeError(error::out_of_bounds);
		}
		return id == 0 ? parent : parent->child_nodes[id - 1].GetLastNode();
	}

	// Returns the very last node of the tree
	//    @return - constant pointer to the last node
	const NodeType* GetLastNode() const
	{
		return child_nodes.Count() == 0 ? static_cast<NodeType*>(this) : child_nodes.GetLast().GetLastNode();
	}

	// Returns the very last node of the tree
	//    @return - non-const pointer to the last node
	NodeType* GetLastNode()
	{
		return child_nodes.Count() == 0 ? static_cast<NodeType*>(this) : child_nodes.GetLast().GetLastNode();
	}

	// Re-assigns id and a parent of the every child node
	inline void ResetChildsId()
	{
		const size_t size = child_nodes.Count();
		for (size_t i = 0; i < size; i++)
		{
			child_nodes[i].id = i;
			child_nodes[i].parent = static_cast<NodeType*>(this);
			child_nodes[i].ResetChildsId();
		}
	}

	// Calculates child node id from iterator
	//    @param iterator - iterator to be converted
	//    @return - id of a child, or nothing if failed
	inline Optional<size_t> ConvertChildIdFromIterator(typename Array<NodeType>::ConstIterator iterator)
	{
		const NodeType* ptr = &(*iterator);
		if (ptr >= child_nodes.GetAddress() && ptr < child_nodes.GetAddress() + child_nodes.Count())
		{
			size_t child_id = ptr - child_nodes.GetAddress();
			return child_id;
		}
		return Optional<size_t>();
	}

	// parent node
	NodeType* parent;

	// id of the node
	size_t id;

	// array of the child nodes
	Array<NodeType> child_nodes;
};

//----------------------------------------------------------------------------//
// ut::Tree template class is a container for an unordered tree data.
template<typename T>
class Tree : public BaseTree<T, Tree<T> >
{
	typedef BaseTree<T, Tree<T> > Base;
public:
	// Constructor, @data has default value
	Tree() : Base()
	{ }

	// Constructor, @data is copied from parameter
	Tree(const typename Base::DataType& data_copy) : Base(data_copy)
	{ }

	// Constructor, @data is moved from parameter
	Tree(typename Base::DataType&& data_rval_ref) : Base(Move(data_rval_ref))
	{ }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//