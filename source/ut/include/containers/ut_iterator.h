//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Iterator Tags
struct ForwardIteratorTag
{	// identifying tag for forward iterators
};

struct BidirectionalIteratorTag : ForwardIteratorTag
{	// identifying tag for bidirectional iterators
};

struct RandomAccessIteratorTag : BidirectionalIteratorTag
{	// identifying tag for random-access iterators
};

//----------------------------------------------------------------------------//
// ut::BaseIterator is a base class, every iterator should be inherited from it.
template<typename IteratorTag,
         typename ValueType,
         typename PointerType,
         typename ReferenceType>
struct BaseIterator
{
	typedef IteratorTag Tag;
	typedef ValueType Type;
	typedef PointerType Pointer;
	typedef ReferenceType Reference;
};

//----------------------------------------------------------------------------//
// Enumeration of noteworthy iterator positions.
namespace iterator
{
	enum Position
	{
		first = 0,
		last = 1,
	};
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//