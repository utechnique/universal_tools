//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "containers/ut_avltree.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::AVLNodeProxy is a helper proxy template class to simplify
// serialization of the AVL trees. Original ut::AVLTree has only raw pointers
// to the left and right leaves, because it's more convenient to rotate nodes
// without smart pointers. However, it's very uncomfortably to serialize data.
// AVLNodeProxy utilizes ut::UniquePtr to serialize child nodes, then exports
// the result to the original tree.
template<typename Key, typename Value>
class AVLNodeProxy : public Reflective
{
	typedef typename AVLTree<Key, Value>::Node Node;
public:
	// Default constructor.
	AVLNodeProxy()
	{ }

	// Constructor, imports data from the provided node.
	//    @param copy - reference to the original node
	//                  representing a source of data.
	AVLNodeProxy(const Node& copy)
	{
		Import(copy);
	}

	// Imports data from the provided node.
	//    @param copy - reference to the original node
	//                  representing a source of data.
	void Import(const Node& copy)
	{
		// import members
		key = copy.GetKey();
		value = copy.value;
		balance = copy.GetBalance();

		// get leaves
		const Node* const left_leaf = copy.GetLeft();
		const Node* const right_leaf = copy.GetRight();

		// import left leaf
		if (left_leaf != nullptr)
		{
			left = MakeUnique<AVLNodeProxy>(*left_leaf);
		}

		// import right leaf
		if (right_leaf != nullptr)
		{
			right = MakeUnique<AVLNodeProxy>(*right_leaf);
		}
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		snapshot.Add(key, "key");
		snapshot.Add(value, "value");
		snapshot.Add(balance, "balance");
		snapshot.Add(left, "left");
		snapshot.Add(right, "right");
	}

	// All members that are present in original
	// avl node are present here too.
	Key key;
	Value value;
	int8 balance;

	// Child nodes are wrapped in UniquePtr to simplify serialization.
	UniquePtr<AVLNodeProxy> left;
	UniquePtr<AVLNodeProxy> right;
};

//----------------------------------------------------------------------------//
// ut::Parameter<AVLTree> is a template specialization for AVL tree container.
template<typename Key, typename Value>
class Parameter< AVLTree<Key, Value> > : public BaseParameter
{
	// type of the original node
	typedef typename AVLTree<Key, Value>::Node Node;

	// type of the proxy node
	typedef AVLNodeProxy<Key, Value> ProxyNode;

	// short type of the current parameter
	typedef Parameter< AVLTree<Key, Value> > AVLParameter;

public:
	// Constructor
	//    @param p - pointer to the managed object
	Parameter(AVLTree<Key, Value>* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< AVLTree<Key, Value> >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// copy avl tree to the temporary proxy structure
		Import();

		// register proxy (not a real tree)
		snapshot.Add(proxy, "root");

		// deserialized proxy will be exported back to the avl tree
		// after loading is done
		snapshot.SetPostLoadCallback(MemberFunction<AVLParameter, void()>(this, &AVLParameter::Export));
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// get array reference from pointer
		AVLTree<Key, Value>& tree = *static_cast<AVLTree<Key, Value>*>(ptr);

		// write value and key type names
		if (controller.GetInfo().HasTypeInformation())
		{
			// value type
			String type_name = BaseParameter::DeduceTypeName<Value>();
			Optional<Error> write_type_error = controller.WriteAttribute(type_name, node_names::skValueType);
			if (write_type_error)
			{
				return write_type_error;
			}

			// key type
			type_name = BaseParameter::DeduceTypeName<Key>();
			write_type_error = controller.WriteAttribute(type_name, node_names::skKeyType);
			if (write_type_error)
			{
				return write_type_error;
			}
		}

		// success
		return Optional<Error>();
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// get array reference from pointer
		AVLTree<Key, Value>& tree = *static_cast<AVLTree<Key, Value>*>(ptr);

		// read value typename and compare with current one
		if (controller.GetInfo().HasTypeInformation())
		{
			// check value type
			Result<String, Error> type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!type_result)
			{
				return type_result.MoveAlt();
			}
			else if (BaseParameter::DeduceTypeName<Value>() != type_result.Get())
			{
				return Error(error::types_not_match);
			}

			// check key type
			type_result = controller.ReadAttribute<String>(node_names::skKeyType);
			if (!type_result)
			{
				return type_result.MoveAlt();
			}
			else if (BaseParameter::DeduceTypeName<Key>() != type_result.Get())
			{
				return Error(error::types_not_match);
			}
		}

		// success
		return Optional<Error>();
	}
	
private:
	// Recursively exports data from the provided proxy node to the original destination node.
	//    @param dst_node - reference to pointer to original node to export data to.
	//    @param proxy_node - const pointer to proxy node to export data from.
	//    @param parent - pointer to the parent of the @dst_node.
	static void ExportNode(Node*& dst_node, const ProxyNode* proxy_node, Node* parent)
	{
		// exit if proxy is empty
		if (proxy_node == nullptr)
		{
			dst_node = nullptr;
			return;
		}

		// create a new node and copy balance value
		dst_node = new Node(proxy_node->key, proxy_node->value, parent);
		dst_node->balance = proxy_node->balance;

		// export both leaves
		ExportNode(dst_node->left, proxy_node->left.Get(), dst_node);
		ExportNode(dst_node->right, proxy_node->right.Get(), dst_node);
	}

	// Imports original tree to the temporary proxy structure.
	void Import()
	{
		// cast pointer to the reference to the avl tree
		AVLTree<Key, Value>& tree = *static_cast<AVLTree<Key, Value>*>(ptr);

		// export tree to the temporary proxy structure
		if (tree.root != nullptr)
		{
			proxy = ut::MakeUnique<ProxyNode>(*tree.root);
		}
	}

	// Exports temporary proxy object to the original tree.
	void Export()
	{
		// cast pointer to the reference to the avl tree
		AVLTree<Key, Value>& tree = *static_cast<AVLTree<Key, Value>*>(ptr);

		// clear current tree
		tree.Empty();

		// export data
		if (proxy)
		{
			ExportNode(tree.root, proxy.Get(), nullptr);
		}
	}

	// AVLTree is too complicated to be serialized "as is", thus a temporary proxy
	// structure is used to simplify serialization process.
	UniquePtr<ProxyNode> proxy;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//