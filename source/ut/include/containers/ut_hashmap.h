//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "containers/ut_array.h"
#include "containers/ut_pair.h"
#include "hash/ut_default_hash.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Node type for the ut::SparseHashMap container.
template<typename KeyType, typename ValueType>
class SparseHashMapNode : public Pair<const KeyType, ValueType>
{
private:
	// Type of the base class whick contains (or not) the key/value pair.
	typedef Pair<const KeyType, ValueType> Base;

	// Members of this node can be accessed only inside the appropriate hashmap.
	template<typename, typename, class, class, class>
	friend class SparseHashMap;

	// Constructor accepts appropriate key/value pair.
	SparseHashMapNode(Pair<const KeyType, ValueType>&& rval) : Base(Move(rval))
	{}

	// Indicates that this node is the last one in the bucket.
	static constexpr size_t end = -1;

	// Index of the next node.
	size_t next_id = end;
};


//----------------------------------------------------------------------------//
// Default functor to compair keys in the hashmap.
template<typename KeyType>
struct DefaultHashMapKeyEqualityFunction
{
	constexpr bool operator()(const KeyType& l, const KeyType& r) const
	{
		return l == r;
	}
};

//----------------------------------------------------------------------------//
// ut::DenseHashMap is an associative container that contains key-value pairs
// with unique keys. Search of the desired element has average constant-time
// complexity. Insertion and removal have O(n) complexity that was sacrificed in
// order to provide the best memory caching (compared to std::unordered_map for
// example) results for the iteration.
template<typename KeyType, 
         typename ValueType,
         class HashFunction = Hash<KeyType>,
         class KeyEqual = DefaultHashMapKeyEqualityFunction<KeyType>,
         class Allocator = DefaultAllocator< Pair<const KeyType, ValueType> > >
class DenseHashMap
{
	typedef Pair<const KeyType, ValueType> Node;

	// Lookup table element type. Lookup table is used for the quick access to the
	// desired element.
	struct LookupNode
	{
		// Index of the first element in the bucket in the hashmap array.
		size_t index;

		// Number of elements in the bucket.
		size_t num;
	};
public:
	// ut::DenseHashMap<>::ConstIterator is a random-access constant iterator to iterate
	// over parent container (ut::DenseHashMap<>). This class is capable only to read
	// the content of the container. Use ut::DenseHashMap<>::Iterator if you want to
	// write (modify) the container data.
	typedef typename Array<Node, Allocator>::ConstIterator ConstIterator;

	// ut::DenseHashMap<>::Iterator is a random-access iterator to iterate over parent
	// container (ut::DenseHashMap<>). This class is the same as ut::DenseHashMap::ConstIterator,
	// but is capable to modify the content of the container.
	typedef typename Array<Node, Allocator>::Iterator Iterator;
	
	// Returns desired element
	Pair<const KeyType, ValueType>& operator [] (const size_t id)
	{
		return nodes[id];
	}

	// Returns desired element
	const Pair<const KeyType, ValueType>& operator [] (const size_t id) const
	{
		return nodes[id];
	}

	// Returns the number of elements in the array
	size_t Count() const
	{
		return nodes.Count();
	}

	// Finds an element with key equivalent to @key
	//    @param key - const reference to the key value
	//    @return - reference to the value, if it was found
	Optional<ValueType&> Find(const KeyType& key)
	{
		// check if there is at least one element
		if (nodes.IsEmpty())
		{
			return Optional<ValueType&>();
		}

		// search for the desired key in the bucket
		const LookupNode& lookup_node = lookup[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			Node& node = nodes[lookup_node.index + i];
			if (equal_function(node.GetFirst(), key))
			{
				return node.second;
			}
		}

		// nothing found
		return Optional<ValueType&>();
	}

	// Finds an element with key equivalent to @key
	//    @param key - const reference to the key value
	//    @return - reference to the value, if it was found
	Optional<const ValueType&> Find(const KeyType& key) const
	{
		// check if there is at least one element
		if (nodes.IsEmpty())
		{
			return Optional<const ValueType&>();
		}

		// search for the desired key in the bucket
		const LookupNode& lookup_node = lookup[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			const Node& node = nodes[lookup_node.index + i];
			if (equal_function(node.GetFirst(), key))
			{
				return node.second;
			}
		}

		// nothing found
		return Optional<const ValueType&>();
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(const KeyType& key,
	                                                 const ValueType& value)
	{
		ReallocAndRehash(nodes.Count() + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(key, value));
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(const KeyType& key,
	                                                 ValueType&& value)
	{
		ReallocAndRehash(nodes.Count() + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(key, Move(value)));
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(KeyType&& key,
	                                                 const ValueType& value)
	{
		ReallocAndRehash(nodes.Count() + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(Move(key), value));
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(KeyType&& key,
	                                                 ValueType&& value)
	{
		ReallocAndRehash(nodes.Count() + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(Move(key), Move(value)));
	}

	// Removes the element with the desired key.
	//    @param key - const reference to the key associated
	//                 with the element to be deleted.
	//    @return - 'true' if the element was found and deleted
	//              or 'false' if there is no such element in the map.
	bool Remove(const KeyType& key)
	{
		// check if there is at least one element
		if (nodes.IsEmpty())
		{
			return false;
		}

		// search for the element
		LookupNode& lookup_node = lookup[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			const size_t remove_position = lookup_node.index + i;
			Node& node = nodes[remove_position];
			if (!equal_function(node.GetFirst(), key))
			{
				continue;
			}

			// destroy the desired element
			nodes.Remove(remove_position);
			lookup_node.num--;

			// shift all keys
			const size_t lookup_size = lookup.Count();
			for (size_t j = 0; j < lookup_size; j++)
			{
				if (lookup[j].index > remove_position)
				{
					lookup[j].index--;
				}
			}

			// element was successfuly found and removed
			return true;
		}

		// element wasn't found
		return false;
	}

	// Destructs all elements and set element count to zero.
	void Reset()
	{
		memory::Set(lookup.GetAddress(), 0, lookup.Count() * sizeof(LookupNode));
		nodes.Reset();
	}

	// Returns the number of collisions in the map.
	size_t GetCollisionCount() const
	{
		size_t collision_count = 0;
		const size_t lookup_size = lookup.Count();
		for (size_t i = 0; i < lookup_size; i++)
		{
			const size_t bucket_size = lookup[i].num;
			if (bucket_size > 1)
			{
				collision_count += bucket_size - 1;
			}
		}
		return collision_count;
	}

	// Returns constant read / write iterator that points to the first element
	ConstIterator Begin(iterator::Position position = iterator::first) const
	{
		return nodes.Begin();
	}

	// Returns constant read / write iterator that points to the last element
	ConstIterator End(iterator::Position position = iterator::last) const
	{
		return nodes.End();
	}

	// Returns a read / write iterator that points to the first element
	Iterator Begin(iterator::Position position = iterator::first)
	{
		return nodes.Begin();
	}

	// Returns a read / write iterator that points to the last element
	Iterator End(iterator::Position position = iterator::last)
	{
		return nodes.End();
	}

private:
	// Returns the index of the node containg information about the
	// (another) node associated with the provided key.
	size_t CalculateLookupId(const KeyType& key) const
	{
		return hash_function(key) % lookup.Count();
	}

	// Inserts new key-value pair to the map.
	//    @param pair - r-value refenrence to the key/value pair.
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added.
	inline Optional<Pair<const KeyType, ValueType>&> EmplacePair(Pair<const KeyType, ValueType>&& pair)
	{
		UT_ASSERT(nodes.Count() < lookup.Count());

		// if there is no key in the map - add this key to the end of the array
		const KeyType& key = pair.GetFirst();
		LookupNode& lookup_node = lookup[CalculateLookupId(key)];
		if (lookup_node.num == 0)
		{
			lookup_node.index = nodes.Count();
			lookup_node.num++;

			nodes.Add(Move(pair));

			return Optional<Pair<const KeyType, ValueType>&>(); // exit
		}
		
		// check if this key already exists
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			if (equal_function(nodes[lookup_node.index + i].GetFirst(), key))
			{
				return nodes[lookup_node.index + i]; // exit
			}
		}

		// insert key to the end of the bucket
		const size_t insert_position = lookup_node.index + lookup_node.num;
		nodes.Insert(insert_position, Move(pair));
		lookup_node.num++;

		// fix lookup table
		const size_t lookup_size = lookup.Count();
		for (size_t i = 0; i < lookup_size; i++)
		{
			if (lookup[i].index >= insert_position)
			{
				lookup[i].index++;
			}
		}

		return Optional<Pair<const KeyType, ValueType>&>();
	}

	// Reallocates memory and recalculates hashes if the
	// provided new size is bigger than the capacity.
	void ReallocAndRehash(size_t new_size)
	{
		if (new_size * max_density_level <= lookup.Count())
		{
			return;
		}
		
		lookup.Resize(new_size * capacity_multiplier);
		memory::Set(lookup.GetAddress(), 0, lookup.Count() * sizeof(LookupNode));
		
		// insert old nodes to the new hashmap
		Array<Node> old_nodes(Move(nodes));
		const size_t old_node_count = old_nodes.Count();
		for (size_t i = 0; i < old_node_count; i++)
		{
			EmplacePair(Move(old_nodes[i]));
		}
	}

	// Hash function object.
	HashFunction hash_function;

	// Equality function.
	KeyEqual equal_function;

	// Memory block allocated for the array.
	Array<Node, Allocator> nodes;
	Array<LookupNode> lookup;

	// Indicates how often the hash table is recalculated.
	static constexpr size_t max_density_level = 1;

	// Indicates how much the capacity is increased after the overflow.
	// The grater this value is - the less collisions occur -> faster search.
	static constexpr size_t capacity_multiplier = 6;
};

//----------------------------------------------------------------------------//
// ut::SparseHashMap is an associative container that contains key-value pairs
// with unique keys. This implementation is similar to the std::unordered_map.
// Search, insert of the desired element has average constant-time complexity.
// Remove() has O(n) complexity for the case when the key hash has collisions.
template<typename KeyType,
         typename ValueType,
         class HashFunction = Hash<KeyType>,
         class KeyEqual = DefaultHashMapKeyEqualityFunction<KeyType>,
         class Allocator = DefaultAllocator< Optional< SparseHashMapNode<KeyType, ValueType> > > >
class SparseHashMap
{
	typedef SparseHashMap<KeyType, ValueType, HashFunction, KeyEqual, Allocator> ThisMap;
	typedef SparseHashMapNode<KeyType, ValueType> Node;
public:
	// ut::SparseHashMap<>::ConstIterator is a random-access constant iterator to iterate
	// over parent container (ut::SparseHashMap<>). This class is capable only to read
	// the content of the container. Use ut::SparseHashMap<>::Iterator if you want to
	// write (modify) the container data.
	class ConstIterator : public BaseIterator<ForwardIteratorTag,
	                                          Pair<const KeyType, ValueType>,
	                                          Pair<const KeyType, ValueType>*,
	                                          Pair<const KeyType, ValueType>&>
	{
		friend class SparseHashMap<KeyType, ValueType, HashFunction, KeyEqual, Allocator>;
	public:
		// Constructor
		ConstIterator() : map(nullptr), id(0)
		{}

		// Constructor
		ConstIterator(ThisMap* hashmap,
		              size_t start_id) : map(hashmap)
		                               , id(start_id)
		{
			if (hashmap == nullptr || id == map->capacity + map->collision_nodes.Count())
			{
				return;
			}

			Optional<Node>& node = map->arr[id];
			if (!node.HasValue())
			{
				this->operator++();
			}
		}

		// Returns constant reference of the managed object
		const Pair<const KeyType, ValueType>& operator*() const
		{
			Optional<Node>& node = id >= map->capacity ?
			                       map->collision_nodes[id - map->capacity] :
			                       map->arr[id];
			return node.Get();
		}

		// Inheritance operator, provides access to the owned object.
		// Return value can't be changed, it must be constant.
		const Pair<const KeyType, ValueType>* operator->() const
		{
			Optional<Node>& node = id >= map->capacity ?
			                       map->collision_nodes[id - map->capacity] :
			                       map->arr[id];
			return &node.Get();
		}

		// Increment operator
		inline ConstIterator& operator++()
		{
			while (true)
			{
				if (++id == map->capacity + map->collision_nodes.Count())
				{
					return *this;
				}

				Optional<Node>& node = id >= map->capacity ?
				                       map->collision_nodes[id - map->capacity] :
				                       map->arr[id];
				if (node.HasValue())
				{
					return *this;
				}
			}
		}

		// Post increment operator
		ConstIterator operator++(int)
		{
			ConstIterator tmp = *this;
			this->operator++();
			return tmp;
		}

		// Comparison operator 'equal to'
		bool operator == (const ConstIterator& right) const
		{
			return id == right.id;
		}

		// Comparison operator 'not equal to'
		bool operator != (const ConstIterator& right) const
		{
			return id != right.id;
		}

	protected:
		ThisMap* map;
		size_t id;
	};

	// ut::SparseHashMap<>::Iterator is a random-access iterator to iterate over parent
	// container (ut::SparseHashMap<>). This class is the same as ut::SparseHashMap::ConstIterator,
	// but is capable to modify the content of the container.
	class Iterator : public ThisMap::ConstIterator
	{
		// Base iterator type
		typedef ConstIterator Base;
	public:
		// Default constructor
		Iterator()
		{}

		// Constructor
		Iterator(ThisMap* hashmap,
		         size_t start_id) : Base(hashmap, start_id)
		{}

		// Copy constructor
		Iterator(const Iterator& copy) : Base(copy)
		{}

		// Assignment operator
		Iterator& operator = (const Iterator& copy)
		{
			Base::operator = (copy);
			return *this;
		}

		// Returns constant reference of the managed object
		Pair<const KeyType, ValueType>& operator*()
		{
			Optional<Node>& node = Base::id >= Base::map->capacity ?
			                       Base::map->collision_nodes[Base::id - Base::map->capacity] :
			                       Base::map->arr[Base::id];
			return node.Get();
		}

		// Inheritance operator, provides access to the owned object.
		// Return value can't be changed, it must be constant.
		Pair<const KeyType, ValueType>* operator->()
		{
			Optional<Node>& node = Base::id >= Base::map->capacity ?
			                       Base::map->collision_nodes[Base::id - Base::map->capacity] :
			                       Base::map->arr[Base::id];
			return &node.Get();
		}
	};

	// Constructor, all members are set to zero.
	SparseHashMap() : arr(nullptr)
	                , num(0)
	                , capacity(0)
	{}

	// Copy constructor, all elements are copied.
	SparseHashMap(const SparseHashMap& copy) : num(copy.num)
	                                         , capacity(copy.capacity)
	                                         , allocator(copy.allocator)
	                                         , arr(allocator.Allocate(copy.capacity))
	                                         , collision_nodes(copy.collision_nodes)
	{
		if (!arr)
		{
			ThrowError(error::out_of_memory);
		}

		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Optional<Node>(copy.arr[i]);
		}
	}

	// Move constructor, all members are swapped.
	SparseHashMap(SparseHashMap&& rval) noexcept : num(rval.num)
	                                             , capacity(rval.capacity)
	                                             , allocator(Move(rval.allocator))
	                                             , arr(rval.arr)
	                                             , collision_nodes(Move(rval.collision_nodes))
	{
		rval.arr = nullptr;
		rval.num = 0;
		rval.capacity = 0;
	}

	// Assignment operator, all elements are copied.
	SparseHashMap& operator = (const SparseHashMap& copy)
	{
		// destruct old array and deallocate memory
		Deallocate(allocator, arr, capacity);

		// copy allocator
		allocator = copy.allocator;

		// copy size counters
		num = copy.num;
		capacity = copy.capacity;

		// allocate new memory
		arr = allocator.Allocate(copy.capacity);
		if (!arr)
		{
			ThrowError(error::out_of_memory);
		}

		// copy all nodes
		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Optional<Node>(copy.arr[i]);
		}
		collision_nodes = copy.collision_nodes;

		return *this;
	}

	// Move operator.
	SparseHashMap& operator = (SparseHashMap&& rval) noexcept
	{
		Deallocate(allocator, arr, capacity);

		num = rval.num;
		capacity = rval.capacity;
		allocator = Move(rval.allocator);
		arr = rval.arr;
		collision_nodes = Move(rval.collision_nodes);
		

		rval.arr = nullptr;
		rval.num = 0;
		rval.capacity = 0;

		return *this;
	}

	// Destructor, destructs all elements and releases memory.
	~SparseHashMap()
	{
		Deallocate(allocator, arr, capacity);
	}

	// Returns the number of elements in the array
	size_t Count() const
	{
		return num;
	}

	// Finds an element with key equivalent to @key
	//    @param key - const reference to the key value
	//    @return - reference to the value, if it was found
	Optional<ValueType&> Find(const KeyType& key)
	{
		// check if there is at least one element
		if (num == 0)
		{
			return Optional<ValueType&>();
		}

		// search for the desired key in the hash table
		Optional<Node>& node = arr[CalculateLookupId(key)];
		if (!node.HasValue())
		{
			// nothing found
			return Optional<ValueType&>();
		}

		// search for the desired key in the list
		return FindCollisionNode(key, node);
	}

	// Finds an element with key equivalent to @key
	//    @param key - const reference to the key value
	//    @return - const reference to the value, if it was found
	Optional<const ValueType&> Find(const KeyType& key) const
	{
		// check if there is at least one element
		if (num == 0)
		{
			return Optional<const ValueType&>();
		}

		// search for the desired key in the hash table
		const Optional<Node>& node = arr[CalculateLookupId(key)];
		if (!node.HasValue())
		{
			// nothing found
			return Optional<const ValueType&>();
		}

		// search for the desired key in the list
		return FindCollisionNode(key, node);
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(const KeyType& key,
	                                                 const ValueType& value)
	{
		ReallocAndRehash(num + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(key, value));
	}

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(const KeyType& key,
	                                                 ValueType&& value)
	{
		ReallocAndRehash(num + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(key, Move(value)));
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(KeyType&& key,
	                                                 const ValueType& value)
	{
		ReallocAndRehash(num + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(Move(key), value));
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	Optional<Pair<const KeyType, ValueType>&> Insert(KeyType&& key,
	                                                 ValueType&& value)
	{
		ReallocAndRehash(num + 1);
		return EmplacePair(Pair<const KeyType, ValueType>(Move(key), Move(value)));
	}

	// Removes the element with the desired key.
	//    @param key - const reference to the key associated
	//                 with the element to be deleted.
	//    @return - 'true' if the element was found and deleted
	//              or 'false' if there is no such element in the map.
	bool Remove(const KeyType& key)
	{
		// check if there is at least one element
		if (num == 0)
		{
			return false;
		}

		// search for the desired key in the hash table
		Optional<Node>& node = arr[CalculateLookupId(key)];
		if (!node.HasValue())
		{
			// nothing found
			return false;
		}

		// check direct hit case
		const size_t child_id = node->next_id;
		if (equal_function(key, node->GetFirst()))
		{
			num--;

			if (child_id == Node::end)
			{
				node = Optional<Node>();
				return true;
			}

			node = Move(collision_nodes[child_id]);
			collision_nodes.Remove(child_id);
			ShiftCollisionNodeLinks(child_id);

			return true;
		}

		// search in the collision list
		return RemoveCollisionNode(key, child_id);
	}

	// Destructs all elements and set element count to zero.
	void Reset()
	{
		for (size_t i = 0; i < capacity; i++)
		{
			arr[i] = Optional<Node>();
		}
		collision_nodes.Reset();
		num = 0;
	}

	// Returns the number of collisions in the map.
	size_t GetCollisionCount() const
	{
		return collision_nodes.Count();
	}

	// Returns constant read / write iterator that points to the first element
	ConstIterator Begin() const
	{
		return ConstIterator(this, 0);
	}

	// Returns constant read / write iterator that points to the last element
	ConstIterator End() const
	{
		return ConstIterator(this, capacity + collision_nodes.Count());
	}

	// Returns a read / write iterator that points to the first element
	Iterator Begin()
	{
		return Iterator(this, 0);
	}

	// Returns a read / write iterator that points to the last element
	Iterator End()
	{
		return Iterator(this, capacity + collision_nodes.Count());
	}

private:
	// Returns the index of the first node with appropriate hash in the bucket.
	size_t CalculateLookupId(const KeyType& key) const
	{
		return hash_function(key) % capacity;
	}

	// Performs recursive search of the provided key.
	//    @param key - const reference to the key value.
	//    @param parent - const reference to the parent node.
	//    @return - const reference to the value, if it was found.
	Optional<const ValueType&> FindCollisionNode(const KeyType& key,
	                                             const Optional<Node>& parent) const
	{
		if (equal_function(key, parent->GetFirst()))
		{
			return parent->second;
		}

		if (parent->next_id != Node::end)
		{
			return FindCollisionNode(key, collision_nodes[parent->next_id]);
		}

		return Optional<const ValueType&>();
	}

	// Performs recursive search of the provided key.
	//    @param key - const reference to the key value.
	//    @param parent - reference to the parent node.
	//    @return - reference to the value, if it was found.
	Optional<ValueType&> FindCollisionNode(const KeyType& key,
	                                       Optional<Node>& parent)
	{
		if (equal_function(key, parent->GetFirst()))
		{
			return parent->second;
		}

		if (parent->next_id != Node::end)
		{
			return FindCollisionNode(key, collision_nodes[parent->next_id]);
		}

		return Optional<ValueType&>();
	}

	// Destructs provided array and deallocates memory.
	static void Deallocate(Allocator& node_allocator,
	                       Optional<Node>* node_arr,
	                       size_t node_count)
	{
		for (size_t i = 0; i < node_count; i++)
		{
			(node_arr + i)->~Optional<Node>();
		}

		if (node_arr)
		{
			node_allocator.Deallocate(node_arr, node_count);
		}
	}

	// Adds a new node if the appropriate hash table position is already busy.
	Optional<Pair<const KeyType, ValueType>&> AddCollisionNode(Optional<Node>& parent,
	                                                           Pair<const KeyType, ValueType>&& pair)
	{
		// check if this key already exists
		if (parent->GetFirst() == pair.GetFirst())
		{
			return parent.Get();
		}

		// go to the end of the tail
		if (parent->next_id != Node::end)
		{
			return AddCollisionNode(collision_nodes[parent->next_id], Move(pair));
		}

		// add node to the last element
		parent->next_id = collision_nodes.Count();
		collision_nodes.Add(Optional<Node>(Move(pair)));
		num++;
		return Optional<Pair<const KeyType, ValueType>&>();
	}

	// Decrements all collision node links if the
	void ShiftCollisionNodeLinks(size_t id)
	{
		for (size_t i = 0; i < capacity; i++)
		{
			Optional<Node>& node = arr[i];
			if (node.HasValue() &&
			    node->next_id != Node::end &&
			    node->next_id > id)
			{
				node->next_id--;
			}
		}

		const size_t collision_count = collision_nodes.Count();
		for (size_t i = 0; i < collision_count; i++)
		{
			Optional<Node>& node = collision_nodes[i];
			if (node->next_id > id)
			{
				node->next_id--;
			}
		}
	}

	// Recursively removes the element with the desired key.
	//    @param key - const reference to the key associated
	//                 with the element to be deleted.
	//    @param node_id - index of the collision node.
	//    @return - 'true' if the element was found and deleted
	//              or 'false' if there is no such element in the map.
	bool RemoveCollisionNode(const KeyType& key, size_t node_id)
	{
		Optional<Node>& node = collision_nodes[node_id];
		const size_t child_id = node->next_id;
		const bool is_last_node = child_id == Node::end;
		if (!equal_function(key, node->GetFirst()))
		{
			return is_last_node ? false : RemoveCollisionNode(key, node_id);
		}

		if (!is_last_node)
		{
			node = Move(collision_nodes[child_id]);
		}

		const size_t remove_id = is_last_node ? node_id : child_id;
		collision_nodes.Remove(remove_id);
		ShiftCollisionNodeLinks(remove_id);
		num--;

		return true;
	}

	// Inserts new key-value pair to the map
	//    @param pair - r-value refenrence to the key/value pair
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	inline Optional<Pair<const KeyType, ValueType>&> EmplacePair(Pair<const KeyType, ValueType>&& pair)
	{
		UT_ASSERT(num < capacity);

		// add pair to the hash table if the appropriate position is empty
		Optional<Node>& first_node = arr[CalculateLookupId(pair.GetFirst())];
		if (!first_node.HasValue())
		{
			first_node = Move(pair);
			num++;
			return Optional<Pair<const KeyType, ValueType>&>();
		}

		// add pair to the end of the list otherwise
		return AddCollisionNode(first_node, Move(pair));
	}

	// Reallocates memory and recalculates hashes if the
	// provided new size is bigger than the capacity.
	void ReallocAndRehash(size_t new_size)
	{
		if (new_size*max_density_level <= capacity)
		{
			return;
		}

		// save old hashmap
		const size_t old_capacity = capacity;
		Optional<Node>* old_arr = arr;
		Array<Optional<Node>, Allocator> old_collision_nodes(Move(collision_nodes));

		// allocate new hashmap
		capacity = new_size * capacity_multiplier;
		arr = allocator.Allocate(capacity);
		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Optional<Node>();
		}

		// insert old nodes to the new hashmap
		num = 0;
		for (size_t i = 0; i < old_capacity; i++)
		{
			Optional<Node>& old_node = old_arr[i];
			if (old_node.HasValue())
			{
				EmplacePair(old_node.Move());
			}
		}

		// insert colided nodes
		const size_t collision_count = old_collision_nodes.Count();
		for (size_t i = 0; i < collision_count; i++)
		{
			EmplacePair(old_collision_nodes[i].Move());
		}

		// release old memory
		Deallocate(allocator, old_arr, old_capacity);
	}

	// Hash function object.
	HashFunction hash_function;

	// Equality function.
	KeyEqual equal_function;

	// Allocator object.
	Allocator allocator;

	// The number of elements in the map.
	size_t num;

	// ut::HashMap allocates more memory than actually needed
	// for perfomance reasons. @capacity represents
	// the real number of elements memory was allocated for.
	size_t capacity;

	// Memory block allocated for the array.
	Optional<Node>* arr;

	// colided nodes
	Array<Optional<Node>, Allocator> collision_nodes;

	// Indicates how often the hash table is recalculated.
	static constexpr size_t max_density_level = 2;

	// Indicates how much the capacity is increased after the overflow.
	// The grater this value is - the less collisions occur -> faster search.
	static constexpr size_t capacity_multiplier = 4;
};

//----------------------------------------------------------------------------//
// ut::DenseHashMap is the default hashmap type.
template<typename KeyType,
         typename ValueType,
         class HashFunction = Hash<KeyType>,
         class KeyEqual = DefaultHashMapKeyEqualityFunction<KeyType>,
         class Allocator = DefaultAllocator< Pair<const KeyType, ValueType> > >
using HashMap = DenseHashMap<KeyType, ValueType, HashFunction, KeyEqual, Allocator>;

//----------------------------------------------------------------------------//
// Specialize type name function for the hashmap container.
template<typename Key, typename Value, class HashFunction, class KeyEqual, class Allocator>
struct Type< DenseHashMap<Key, Value, HashFunction, KeyEqual, Allocator> >
{
	static inline const char* Name() { return "hashmap"; }
};

template<typename Key, typename Value, class HashFunction, class KeyEqual, class Allocator>
struct Type< SparseHashMap<Key, Value, HashFunction, KeyEqual, Allocator> >
{
	static inline const char* Name() { return "hashmap"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//