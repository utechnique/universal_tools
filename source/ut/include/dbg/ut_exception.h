//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "templates/ut_function.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Handles std::exception and ut::Error exceptions thrown by the provided
// function. In case any of these was catched - writes message to log.
//    @param proc - function to be called and handled.
//    @return - 'true' if catched an exception, 'false' otherwise.
bool CatchExceptions(ut::Function<void()> proc);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//