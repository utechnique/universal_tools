//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_iterator.h"
#include "error/ut_throw_error.h"
#include "templates/ut_is_copy_constructible.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::BaseArray is a parent container for dynamic arrays, you can implement
// your container derived from it to override desired features.
template<typename T>
class BaseArray
{
typedef T ElementType;
public:
	// ut::Array<>::ConstIterator is a random-access constant iterator to iterate
	// over parent container (ut::Array<>). This class is capable only to read
	// the content of the container. Use ut::Array<>::Iterator if you want to
	// write (modify) the container data.
	class ConstIterator : public BaseIterator<RandomAccessIteratorTag, T, T*, T&>
	{
		friend class BaseArray<T>;
	public:
		// Default constructor
		ConstIterator() : ptr(nullptr)
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		ConstIterator(T* p) : ptr(p)
		{ }

		// Copy constructor
		ConstIterator(const ConstIterator& copy) : ptr(copy.ptr)
		{ }

		// Assignment operator
		ConstIterator& operator = (const ConstIterator& copy)
		{
			ptr = copy.ptr;
			return *this;
		}

		// Returns constant reference of the managed object
		const T& operator*() const
		{
			return (*ptr);
		}

		// Inheritance operator, provides access to the owned object.
		// Return value can't be changed, it must be constant.
		const T* operator->() const
		{
			return ptr;
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
		T* ptr;
	};

	// ut::Array<>::Iterator is a random-access iterator to iterate over parent
	// container (ut::Array<>). This class is the same as ut::Array::ConstIterator,
	// but is capable to modify the content of the container.
	class Iterator : public BaseArray<T>::ConstIterator
	{
		// Base iterator type
		typedef ConstIterator Base;
	public:
		// Default constructor
		Iterator()
		{ }

		// Constructor
		//    @param p - initialize iterator with this pointer
		Iterator(T* p) : Base(p)
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
		T& operator*()
		{
			return (*Base::ptr);
		}

		// Inheritance operator, provides access to the owned object.
		T* operator->()
		{
			return Base::ptr;
		}
	};

	// Default constructor
	BaseArray() : arr(nullptr)
			    , num(0)
			    , reserved_elements(0)
	{ }

	// Constructor, creates @num_elements new empty elements
	//    @param num_elements - how many elements to be initialized
	BaseArray(size_t num_elements) : arr(nullptr)
								   , num(0)
								   , reserved_elements(0)
	{
		if (!AllocEmpty(num_elements))
		{
			ThrowError(error::out_of_memory);
		}
	}

	// Constructor, copies @num_elements elements from the provided raw array
	//    @param num_elements - how many elements to be copied
	//    @param ptr - pointer to the first element
	BaseArray(size_t num_elements, const T* ptr) : arr(nullptr)
	                                             , num(0)
	                                             , reserved_elements(0)
	{
		num = num_elements;
		arr = nullptr;
		if (Realloc())
		{
			for (size_t i = 0; i < num; i++)
			{
				new(arr + i) ElementType(ptr[i]);
			}
		}
		else
		{
			ThrowError(error::out_of_memory);
		}
	}

	// Swap constructor, doesn't allocate new memory. It's needed to construct
	// array from another released array. DON'T use this constructor manually!
	BaseArray(const T* ptr,
	          size_t num_elements,
	          size_t reserved) : arr(ptr)
	                           , num(num_elements)
	                           , reserved_elements(reserved)
	{ }

	// Constructor, copies content of another array
	//    @param copy - array to copy
	BaseArray(const BaseArray& copy) : arr(nullptr)
	                                 , num(0)
	                                 , reserved_elements(0)
	{
		if (!CopyToEmpty(copy))
		{
			ThrowError(error::out_of_memory);
		}
	}

	// Constructor, moves content of another array
	//    @param copy - array to copy

	BaseArray(BaseArray&& other) noexcept : arr(other.arr)
	                                      , num(other.num)
	                                      , reserved_elements(other.reserved_elements)
	{
		other.arr = nullptr;
		other.num = 0;
		other.reserved_elements = 0;
	}

	// Assignment operator
	//    @param copy - array to copy
	BaseArray& operator = (const BaseArray& copy)
	{
		Empty();
		if (!CopyToEmpty(copy))
		{
			ThrowError(error::out_of_memory);
		}
		return *this;
	}

	// Assignment (move) operator, moves content of another array
	//    @param other - array to move
	BaseArray& operator = (BaseArray&& other) noexcept
	{
		// release memory
		Empty();

		// move
		arr = other.arr;
		num = other.num;
		reserved_elements = other.reserved_elements;

		// swap
		other.arr = nullptr;
		other.num = 0;
		other.reserved_elements = 0;

		return *this;
	}

	// Returns desired element
	ElementType& operator [] (const size_t id)
	{
		return id < num ? arr[id] : arr[num - 1];
	}

	// Returns desired element
	const ElementType& operator [] (const size_t id) const
	{
		return id < num ? arr[id] : arr[num - 1];
	}

	// Returns a pointer to desired element
	ElementType* operator () (const size_t id)
	{
		return id < num ? &arr[id] : &arr[num - 1];
	}

	// Additive promotion operator
	BaseArray operator +(const BaseArray& other) const
	{
		BaseArray out(*this);
		out += other;
		return out;
	}

	// Additive promotion operator, r-value variant
	BaseArray operator +(BaseArray&& other) const
	{
		BaseArray out(*this);
		out += Move(other);
		return out;
	}

	// Addition assignment operator
	BaseArray& operator +=(const BaseArray& other)
	{
		if (!Concatenate<const ElementType&>(other))
		{
			ThrowError(ut::error::out_of_memory);
		}
		return *this;
	}

	// Addition assignment operator, r-value variant
	BaseArray& operator +=(BaseArray&& other)
	{
		if (!Concatenate<ElementType&&>(Move(other)))
		{
			ThrowError(ut::error::out_of_memory);
		}
		return *this;
	}

	// Destructor, releases allocated memory
	~BaseArray()
	{
		Empty();
	}

	// Returns the size of the array in bytes
	inline size_t GetSize() const
	{
		return num * sizeof(ElementType);
	}

	// Returns the number of elements in the array
	inline size_t GetNum() const
	{
		return num;
	}

	// Returns 'true' if @num is null
	bool IsEmpty() const
	{
		return num == 0;
	}
	
	// Returns pointer to the first element
	ElementType *GetAddress() 
	{
		return arr;
	}

	// Returns pointer to the first element
	const ElementType *GetAddress() const
	{
		return arr;
	}
		
	// Changes the size of array, can crop the ending, or add new elements
	// depending on case if @num_elements is greater than @num or not
	//    @param num_elements - new size of the array, in elements
	//    @return - 'true' if successful, or 'false' if not enough memory
	bool Resize(size_t num_elements)
	{
		size_t old_num = num;
		num = num_elements;
		if (Realloc())
		{
			// construct new elements
			for (size_t i = old_num; i < num; i++)
			{
				PlacementNew(i);
			}

			// success
			return true;
		}
		else
		{
			// not enough memory
			num = 0;
			return false;
		}
	}
	
	// Changes the size of array, can crop the ending or add new elements
	// depending on case if @num_elements is greater than @num or not
	//    @param num_elements - new size of the array, in elements
	bool AllocEmpty(size_t num_elements)
	{
		Empty();
			
		num = num_elements;

		if (Realloc())
		{
			for (size_t i = 0; i < num; i++)
			{
				PlacementNew(i);
			}	
			return true;
		}
		else
		{
			return false;
		}
	}

	// Adds new element to the end of the array (r-value reference)
	// uses move semantics to perform copying
	//    @param element - r-value referece to a new element
	bool Add(ElementType&& element)
	{
		num++;
			
		if (Realloc())
		{
			Emplace(Move(element), num - 1);
			return true;
		}
		else
		{
			num--;
			return false;
		}
	}

	// Adds new element to the end of the array (reference), @copy is a constant
	//    @param copy - new element
	bool Add(const ElementType& copy)
	{
		num++;
			
		if (Realloc())
		{
			Emplace(copy, num - 1);
			return true;
		}
		else
		{
			num--;
			return false;
		}
	}
	
	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param element - element to be moved
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(size_t position, ElementType&& element)
	{
		if (position >= num)
		{
			return false;
		}
		return EmplaceForward(position, Move(element));
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param copy - element to be copied
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(size_t position, const ElementType& copy)
	{
		if (position >= num)
		{
			return false;
		}
		return EmplaceForward(position, copy);
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param element - element to be moved
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(ConstIterator position, ElementType&& element)
	{
		T* ptr = position.ptr;
		if (ptr >= arr && ptr < arr + num)
		{
			size_t id = ptr - arr;
			return EmplaceForward(id, Move(element));
		}
		return false;
	}

	// Inserts elements at the specified location in the container.
	//    @param position - iterator before which the content will be inserted
	//    @param copy - element to be copied
	//    @return - 'true' if element was inserted successfully
	//              'false' if not enough memory, or @position is out of range
	bool Insert(ConstIterator position, const ElementType& copy)
	{
		T* ptr = position.ptr;
		if (ptr >= arr && ptr < arr + num)
		{
			size_t id = ptr - arr;
			return EmplaceForward(id, Move(copy));
		}
		return false;
	}

	// Moves an element in front of the array
	//    @param copy - element to be added
	//    @return - 'true' if element was added successfully
	//              'false' if not enough memory
	bool PushForward(ElementType&& copy)
	{
		return EmplaceForward(0, Move(copy));
	}

	// Adds an element in front of the array
	//    @param copy - element to be added
	//    @return - 'true' if element was added successfully
	//              'false' if not enough memory
	bool PushForward(const ElementType& copy)
	{
		return EmplaceForward(0, copy);
	}
	
	// Removes last element
	void PopBack()
	{
		num--;
		Destruct(num);
		Realloc();
	}
	
	// Removes desired element
	//    @param id - element's index
	void Remove(size_t id)
	{
		// id can't be higher than array size
		if (id >= num)
			return;
		
		// destruct specified element
		Destruct(id);

		// shift all elements to the left
		for (size_t i = id+1; i < num; i++)
		{	
			new(arr + i - 1) ElementType(Move(arr[i]));
			Destruct(i);
		}

		// decrement size
		num--;

		// reallocate memory
		Realloc();
	}
	
	// Removes desired element
	//    @param iterator - element's position
	void Remove(ConstIterator iterator)
	{
		T* ptr = iterator.ptr;
		if (ptr >= arr && ptr < arr + num)
		{
			size_t id = ptr - arr;
			Remove(id);
		}
	}

	// Releases previously allocated memory
	void Empty()
	{
		if (num)
		{
			for (size_t i = 0; i < num; i++)
			{
				Destruct(i);
			}
				
			free(arr);
			arr = nullptr;
			reserved_elements = 0;
				
			num = 0;
		}
	}
	
	// Returns first element
	ElementType& GetFirst()
	{
		return arr[0];
	}

	// Returns first element
	const ElementType& GetFirst() const
	{
		return arr[0];
	}

	// Returns last element
	ElementType& GetLast()
	{
		return arr[num - 1];
	}

	// Returns last element
	const ElementType& GetLast() const
	{
		return arr[num - 1];
	}

	// Moves desired element
	ElementType&& MoveElement(size_t id)
	{
		UT_ASSERT(id < num);
		return Move(arr[id]);
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

protected:
	// Uses 'placement new' to initialize element
	//    @id - index of element to initialize
	inline void PlacementNew(size_t id)
	{
		new(arr + id) ElementType();
	}
	
	// Uses 'placement new' to initialize element
	//    @arg - r-value reference of the object to be initialized
	//    @id - index of the element to be initialized
	template <typename ArgType>
	inline void Emplace(ArgType&& arg, size_t id)
	{
		new(arr + id) ElementType(Forward<ArgType>(arg));
	}

	// Adds an element in front of the array
	//    @param element - element to be added
	//    @return - 'true' if element was added successfully
	//              'false' if not enough memory
	inline bool EmplaceForward(size_t position, ElementType&& element)
	{
		num++;
		if (Realloc())
		{
			// shift all elements forward
			size_t last = num - 1;
			size_t next = position + 1;
			for (size_t i = last; i >= next; i--)
			{
				new(arr + i) ElementType(Move(arr[i - 1]));
				Destruct(i - 1);
			}

			// emplace new element
			new(arr + position) ElementType(Forward<ElementType>(element));

			// success
			return true;
		}
		else
		{
			num--;
			return false;
		}
	}

	// Concatenates current data and contents of another array
	//    @param other - array to copy (or move) content from
	template <typename ElementRefType, typename ArgRefType>
	inline bool Concatenate(ArgRefType other)
	{
		const size_t previous_num = num;
		const size_t other_num = other.GetNum();
		num += other_num;

		// allocate memory for new elements
		if (!Realloc())
		{
			return false;
		}

		// copy (move) tail
		for (size_t i = 0; i < other_num; i++)
		{
			Emplace(static_cast<ElementRefType>(other[i]), previous_num + i);
		}

		// success
		return true;
	}

	// Destructs desired element
	//    @id - element's index
	inline void Destruct(size_t id)
	{
		(arr + id)->~ElementType();
	}

	// Copies content of another array, without calling ut::Array::Empty()
	//    @param copy - array to copy content from
	bool CopyToEmpty(const BaseArray& copy)
	{
		num = copy.GetNum();
		arr = nullptr;

		if (Realloc())
		{
			for (size_t i = 0; i < num; i++)
			{
				new(arr + i) ElementType(copy[i]);
			}
			return true;
		}
		else
		{
			num = 0;
			return false;
		}
	}

	// Realloc() function performs array data reallocation and new memory 
	// block will have size = @reserved_elements * sizeof(ElementType)
	//    @return - 'true' if successful, 'false' if not enough memory
	bool Realloc()
	{
		if (EnoughReservedPlace())
		{
			return true;
		}
		else
		{
			if (num)
			{
				bool needs_realloc = num >= reserved_elements || num <= reserved_elements / 4;

				if (needs_realloc)
				{
					// reserve double size of elements
					reserved_elements = num * 2;

					ElementType* pmem = (ElementType*)realloc(arr, sizeof(ElementType) * reserved_elements);
					if (pmem)
					{
						arr = pmem;
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					// reserved space is still enough for new elements
					return true;
				}
			}
			else
			{
				if (arr)
				{
					free(arr);
				}

				arr = nullptr;
				reserved_elements = 0;

				return true;
			}
		}
	}

	// Checks if @number is out of the available reserved space
	bool EnoughReservedPlace() const
	{
		return num > reserved_elements ? false : true;
	}

	// memory block allocated for the array
	ElementType *arr;

	// number of elements in the array
	size_t       num;

	// ut::Array allocates more memory than actually needed
	// for perfomance reasons. @reserved_elements represents
	// the real number of elements, mem was allocated for.
	size_t       reserved_elements;
};

// ut::Array is a container that encapsulates dynamic arrays
template<typename T>
class Array : public BaseArray<T>
{
typedef T ElementType;
typedef BaseArray<T> Base;
public:
	// Default constructor
	Array()
	{}

	// Constructor, creates @num_elements new empty elements
	//    @param num_elements - how many elements to be initialized
	Array(size_t num_elements) : Base(num_elements)
	{}

	// Constructor, copies content of another array
	//    @param copy - array to copy
	Array(const Array& copy) : Base(copy)
	{}

	// Constructor, moves content of another array
	//    @param copy - array to copy
	Array(Array&& other) noexcept : Base(Move(other))
	{}

	// Assignment operator
	//    @param copy - array to copy
	Array& operator = (const Array& copy)
	{
		Base::operator = (copy);
		return *this;
	}

	// Assignment (move) operator, moves content of another array
	//    @param copy - array to copy
	Array& operator = (Array&& other) noexcept
	{
		return static_cast<Array&>(Base::operator = (Move(other)));
	}

	// Additive promotion operator
	Array operator +(const Array& other) const
	{
		Array out(*this);
		out += other;
		return out;
	}

	// Additive promotion operator, r-value variant
	Array operator +(Array&& other) const
	{
		Array out(*this);
		out += Move(other);
		return out;
	}

	// Addition assignment operator
	Array& operator +=(const Array& other)
	{
		return static_cast<Array&>(Base::operator += (other));
	}

	// Addition assignment operator, r-value variant
	Array& operator +=(Array&& other)
	{
		return static_cast<Array&>(Base::operator += (Move(other)));
	}
};

//----------------------------------------------------------------------------//
// Specialize type name function for arrays
template <typename T> struct Type< Array<T> >
{
	static inline const char* Name() { return "array"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//