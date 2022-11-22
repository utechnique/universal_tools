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
// This is a node type of the appropriate hashmap container. It must not be used
// anyhow outside of the ut::HashMap template class.
template<typename Key, typename Value>
class HashMapNode : public Optional< Pair<const Key, Value> >
{
private:
	// Type of the base class whick contains (or not) the key/value pair.
	typedef Optional< Pair<const Key, Value> > Base;

	// Members of this node can be accessed only inside the appropriate hashmap.
	template<typename, typename, class, class, class>
	friend class HashMap;

	// Index of the first element in the bucket in the hashmap array.
	size_t index = 0;

	// Number of elements in the bucket.
	size_t num = 0;
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
// ut::HashMap is an associative container that contains key-value pairs with
// unique keys. Search of the desired element has average constant-time
// complexity. Insertion and removal have O(n) complexity that was sacrificed in
// order to provide the best memory caching (compared to std::unordered_map for
// example) results for searching and iteration.
template<typename KeyType, 
         typename ValueType,
         class HashFunction = Hash<KeyType>,
         class KeyEqual = DefaultHashMapKeyEqualityFunction<KeyType>,
         class Allocator = DefaultAllocator< HashMapNode<KeyType, ValueType> > >
class HashMap
{
	typedef HashMap<KeyType, ValueType, HashFunction, KeyEqual, Allocator> ThisMap;
	typedef HashMapNode<KeyType, ValueType> Node;
public:
	// ut::HashMap<>::ConstIterator is a random-access constant iterator to iterate
	// over parent container (ut::HashMap<>). This class is capable only to read
	// the content of the container. Use ut::HashMap<>::Iterator if you want to
	// write (modify) the container data.
	class ConstIterator : public BaseIterator<RandomAccessIteratorTag,
	                                          Node, Node*, Node&>
	{
		friend class HashMap<KeyType, ValueType, HashFunction, KeyEqual, Allocator>;
	public:
		// Default constructor
		ConstIterator() : ptr(nullptr)
		{}

		// Constructor
		//    @param p - initialize iterator with this pointer
		ConstIterator(Node* p) : ptr(p)
		{}

		// Copy constructor
		ConstIterator(const ConstIterator& copy) : ptr(copy.ptr)
		{}

		// Assignment operator
		ConstIterator& operator = (const ConstIterator& copy)
		{
			ptr = copy.ptr;
			return *this;
		}

		// Returns constant reference of the managed object
		const Pair<const KeyType, ValueType>& operator*() const
		{
			return ptr->Get();
		}

		// Inheritance operator, provides access to the owned object.
		// Return value can't be changed, it must be constant.
		const Pair<const KeyType, ValueType>* operator->() const
		{
			return &ptr->Get();
		}

		// Increment operator
		ConstIterator& operator++()
		{
			ptr++;
			return *this;
		}

		// Post increment operator
		ConstIterator operator++(int)
		{
			ConstIterator tmp = *this;
			ptr++;
			return tmp;
		}

		// Decrement operator
		ConstIterator& operator--()
		{
			ptr--;
			return *this;
		}

		// Post decrement operator
		ConstIterator operator--(int)
		{
			ConstIterator tmp = *this;
			ptr--;
			return tmp;
		}

		// Shifts iterator forward
		//    @param offset - offset in elements
		ConstIterator& operator += (size_t offset)
		{
			ptr += offset;
			return *this;
		}

		// Shifts iterator forward (not modifying it) and returns the result
		//    @param offset - offset in elements
		ConstIterator operator + (size_t offset) const
		{
			ConstIterator tmp = *this;
			return (tmp += offset);
		}

		// Shifts iterator backward
		//    @param offset - offset in elements
		ConstIterator& operator -= (size_t offset)
		{
			ptr -= offset;
			return *this;
		}

		// Shifts iterator backward (not modifying it) and returns the result
		//    @param offset - offset in elements
		ConstIterator operator - (size_t offset) const
		{
			ConstIterator tmp = *this;
			return (tmp -= offset);
		}

		// Shifts iterator forward (not modifying it) and returns the result
		//    @param offset - offset in elements
		ConstIterator& operator[](size_t offset) const
		{
			return (*(*this + offset));
		}

		// Comparison operator 'less than'
		bool operator < (const ConstIterator& right) const
		{
			return (ptr < right.ptr);
		}

		// Comparison operator 'less than or equal'
		bool operator <= (const ConstIterator& right) const
		{
			return (ptr <= right.ptr);
		}

		// Comparison operator 'greater than'
		bool operator > (const ConstIterator& right) const
		{
			return (ptr > right.ptr);
		}

		// Comparison operator 'greater than or equal'
		bool operator >= (const ConstIterator& right) const
		{
			return (ptr >= right.ptr);
		}

		// Comparison operator 'equal to'
		bool operator == (const ConstIterator& right) const
		{
			return ptr == right.ptr;
		}

		// Comparison operator 'not equal to'
		bool operator != (const ConstIterator& right) const
		{
			return ptr != right.ptr;
		}

	protected:
		// managed object
		Node* ptr;
	};

	// ut::HashMap<>::Iterator is a random-access iterator to iterate over parent
	// container (ut::HashMap<>). This class is the same as ut::HashMap::ConstIterator,
	// but is capable to modify the content of the container.
	class Iterator : public ThisMap::ConstIterator
	{
		// Base iterator type
		typedef ConstIterator Base;
	public:
		// Default constructor
		Iterator()
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		Iterator(Node* p) : Base(p)
		{ }

		// Copy constructor
		Iterator(const Iterator& copy) : Base(copy)
		{ }

		// Assignment operator
		Iterator& operator = (const Iterator& copy)
		{
			Base::operator = (copy);
			return *this;
		}

		// Returns reference of the managed object
		Pair<const KeyType, ValueType>& operator*()
		{
			return Base::ptr->Get();
		}

		// Inheritance operator, provides access to the owned object.
		Pair<const KeyType, ValueType>* operator->()
		{
			return &Base::ptr->Get();
		}
	};

	// Constructor, all members are set to zero.
	HashMap() : arr(nullptr)
	          , num(0)
	          , capacity(0)
	{}

	// Copy constructor, all elements are copied.
	HashMap(const HashMap& copy) : allocator(copy.allocator)
	                             , arr(allocator.Allocate(copy.capacity))
	                             , num(copy.num)
	                             , capacity(copy.capacity)
	{
		if (!arr)
		{
			ThrowError(error::out_of_memory);
		}

		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Node(copy.arr[i]);
		}
	}

	// Move constructor, all members are swapped.
	HashMap(HashMap&& rval) noexcept : allocator(Move(rval.allocator))
	                                 , arr(rval.arr)
	                                 , num(rval.num)
	                                 , capacity(rval.capacity)
	{
		rval.arr = nullptr;
		rval.num = 0;
		rval.capacity = 0;
	}

	// Assignment operator, all elements are copied.
	HashMap& operator = (const HashMap& copy)
	{
		// destruct old array and deallocate memory
		Deallocate(allocator, arr, capacity);

		// copy allocator
		allocator = copy.allocator;

		// allocate new memory
		arr = allocator.Allocate(copy.capacity);
		if (!arr)
		{
			ThrowError(error::out_of_memory);
		}

		// copy other members
		num = copy.num;
		capacity = copy.capacity;

		// copy all nodes
		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Node(copy.arr[i]);
		}

		return *this;
	}

	// Move operator.
	HashMap& operator = (HashMap&& rval) noexcept
	{
		allocator = Move(rval.allocator);

		arr = rval.arr;
		num = rval.num;
		capacity = rval.capacity;

		rval.arr = nullptr;
		rval.num = 0;
		rval.capacity = 0;

		return *this;
	}

	// Destructor, destructs all elements and releases memory.
	~HashMap()
	{
		Deallocate(allocator, arr, capacity);
	}
	
	// Returns desired element
	Pair<const KeyType, ValueType>& operator [] (const size_t id)
	{
		return arr[id].Get();
	}

	// Returns desired element
	const Pair<const KeyType, ValueType>& operator [] (const size_t id) const
	{
		return arr[id].Get();
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

		// search for the desired key in the bucket
		const Node& lookup_node = arr[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			Node& node = arr[lookup_node.index + i];
			if (equal_function(node->GetFirst(), key))
			{
				return node->second;
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
		if (num == 0)
		{
			return Optional<const ValueType&>();
		}

		// search for the desired key in the bucket
		const Node& lookup_node = arr[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			const Node& node = arr[lookup_node.index + i];
			if (equal_function(node->GetFirst(), key))
			{
				return node->second;
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
		ReallocAndRehash(num + 1);
		return EmplacePair(key, value);
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
		return EmplacePair(key, Move(value));
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
		return EmplacePair(Move(key), value);
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
		return EmplacePair(Move(key), Move(value));
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

		// search for the element
		Node& lookup_node = arr[CalculateLookupId(key)];
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			const size_t remove_position = lookup_node.index + i;
			Node& node = arr[remove_position];
			if (!equal_function(node->GetFirst(), key))
			{
				continue;
			}

			// destroy the desired element
			node.Node::Base::operator = (Optional< Pair<const KeyType, ValueType> >());
			lookup_node.num--;
			num--;

			// shift all keys
			for (size_t j = 0; j < capacity; j++)
			{
				if (j > remove_position)
				{
					arr[j - 1].Node::Base::operator = (Move(arr[j]));
				}

				if (arr[j].index > remove_position)
				{
					arr[j].index--;
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
		for (size_t i = 0; i < capacity; i++)
		{
			Node& node = arr[i];
			node.Node::Base::operator = (Optional< Pair<const KeyType, ValueType> >());
			node.num = 0;
		}
		num = 0;
	}

	// Returns the number of collisions in the map.
	size_t GetCollisionCount() const
	{
		size_t collision_count = 0;
		for (size_t i = 0; i < capacity; i++)
		{
			const size_t bucket_size = arr[i].num;
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
		return position == iterator::first ? ConstIterator(arr) : ConstIterator(arr + num - 1);
	}

	// Returns constant read / write iterator that points to the last element
	ConstIterator End(iterator::Position position = iterator::last) const
	{
		return position == iterator::last ? ConstIterator(arr + num) : ConstIterator(arr - 1);
	}

	// Returns a read / write iterator that points to the first element
	Iterator Begin(iterator::Position position = iterator::first)
	{
		return position == iterator::first ? Iterator(arr) : Iterator(arr + num - 1);
	}

	// Returns a read / write iterator that points to the last element
	Iterator End(iterator::Position position = iterator::last)
	{
		return position == iterator::last ? Iterator(arr + num) : Iterator(arr - 1);
	}

private:
	// Returns the index of the node containg information about the
	// (another) node associated with the provided key.
	size_t CalculateLookupId(const KeyType& key) const
	{
		return hash_function(key) % capacity;
	}

	// Destructs provided array and deallocates memory.
	static void Deallocate(Allocator& node_allocator,
	                       Node* node_arr,
	                       size_t node_count)
	{
		for (size_t i = 0; i < node_count; i++)
		{
			(node_arr + i)->~Node();
		}

		if (node_arr)
		{
			node_allocator.Deallocate(node_arr, node_count);
		}
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - optional key/value pair if such key already exists,
	//              or nothing if successfully added
	template <typename ArgType1, typename ArgType2>
	inline Optional<Pair<const KeyType, ValueType>&> EmplacePair(ArgType1&& key,
	                                                             ArgType2&& value)
	{
		UT_ASSERT(num < capacity);

		// if there is no key in the map - add this key to to the end of the array
		Node& lookup_node = arr[CalculateLookupId(key)];
		if (lookup_node.num == 0)
		{
			arr[num].Node::Base::operator = (Pair<const KeyType, ValueType>(Forward<ArgType1>(key),
			                                                                Forward<ArgType2>(value)));
			lookup_node.index = num;
			lookup_node.num++;
			num++;

			return Optional<Pair<const KeyType, ValueType>&>(); // exit
		}
		
		// check if this key already exists
		for (size_t i = 0; i < lookup_node.num; i++)
		{
			if (arr[lookup_node.index + i]->GetFirst() == key)
			{
				return arr[lookup_node.index + i].Get(); // exit
			}
		}

		// shift all elements forward
		const size_t insert_position = lookup_node.index + lookup_node.num;
		for (size_t i = capacity; i-- > 0;)
		{
			if (i > 0 && i > insert_position)
			{
				arr[i].Node::Base::operator = (Move(arr[i - 1]));
			}

			if (arr[i].index >= insert_position)
			{
				arr[i].index++;
			}
		}

		// insert key to the end of the bucket
		arr[insert_position].Node::Base::operator = (Pair<const KeyType, ValueType>(Forward<ArgType1>(key),
		                                                                            Forward<ArgType2>(value)));
		lookup_node.num++;
		num++;

		return Optional<Pair<const KeyType, ValueType>&>();
	}

	// Reallocates memory and recalculates hashes if the
	// provided new size is bigger than the capacity.
	void ReallocAndRehash(size_t new_size)
	{
		if (new_size <= capacity)
		{
			return;
		}
		
		// save old hashmap
		const size_t old_num = num;
		const size_t old_capacity = capacity;
		Node* old_arr = arr;

		// allocate new hashmap
		capacity = new_size * capacity_multiplier;
		arr = allocator.Allocate(capacity);
		for (size_t i = 0; i < capacity; i++)
		{
			new(arr + i) Node();
		}

		// insert old nodes to the new hashmap
		num = 0;
		for (size_t i = 0; i < old_num; i++)
		{
			EmplacePair(old_arr[i]->GetFirst(), old_arr[i]->second);
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

	// Memory block allocated for the array.
	Node* arr;

	// The number of elements in the map.
	size_t num;

	// ut::HashMap allocates more memory than actually needed
	// for perfomance reasons. @capacity represents
	// the real number of elements memory was allocated for.
	size_t capacity;

	// Indicates how much the capacity is increased after the overflow.
	// The grater this value is - the less collisions occur -> faster search.
	static constexpr size_t capacity_multiplier = 3;
};

//----------------------------------------------------------------------------//
// Specialize type name function for the hashmap container.
template<typename Key, typename Value, class HashFunction, class KeyEqual, class Allocator>
struct Type< HashMap<Key, Value, HashFunction, KeyEqual, Allocator> >
{
	static inline const char* Name() { return "hashmap"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//