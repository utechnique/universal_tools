//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "templates/ut_optional.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Returns an iterator to the first element in the range [first, last) equal to
// the provided @value (or nothing if there is no such iterator).
template<class IteratorType, class T>
Optional<IteratorType> Find(IteratorType first,
                            IteratorType last,
                            const T& value)
{
    for (; first != last; ++first)
    {
        if (*first == value)
        {
            return first;
        }
    }

    return Optional<IteratorType>();
}

// Returns an iterator to the first element in the range [first, last) that
// satisfies specific criteria (or nothing if there is no such iterator).
template<class IteratorType, class UnaryPredicate>
Optional<IteratorType> FindIf(IteratorType first,
                              IteratorType last,
                              UnaryPredicate p)
{
    for (; first != last; ++first)
    {
        if (p(*first))
        {
            return first;
        } 
    }

    return Optional<IteratorType>();
}

// Returns an iterator to the first element in the range [first, last) that
// satisfies specific criteria (or nothing if there is no such iterator).
template<class IteratorType, class UnaryPredicate>
Optional<IteratorType> FindIfNot(IteratorType first,
                                 IteratorType last,
                                 UnaryPredicate p)
{
    for (; first != last; ++first)
    {
        if (!p(*first))
        {
            return first;
        } 
    }

    return Optional<IteratorType>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//