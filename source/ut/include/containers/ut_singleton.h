//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Singleton is a class for 'singleton' pattern.
// Implementation of this class was inspired by
// boost::singleton_default<> class.
// Here is a tech doc from boost library:
//
// The following helper classes are placeholders for a generic "singleton"
//  class.  The classes below support usage of singletons, including use in
//  program startup/shutdown code, AS LONG AS there is only one thread
//  running before main() begins, and only one thread running after main()
//  exits.
//
// This class is also limited in that it can only provide singleton usage for
//  classes with default constructors.
//
// The design of this class is somewhat twisted, but can be followed by the
//  calling inheritance.  Let us assume that there is some user code that
//  calls "ut::Singleton<T>::Instance()".  The following (convoluted)
//  sequence ensures that the same function will be called before main():
//    Instance() contains a call to create_object.do_nothing()
//    Thus, object_creator is implicitly instantiated, and create_object
//      must exist.
//    Since create_object is a static member, its constructor must be
//      called before main().
//    The constructor contains a call to instance(), thus ensuring that
//      Instance() will be called before main().
//    The first time Instance() is called (i.e., before main()) is the
//      latest point in program execution where the object of type T
//      can be created.
//    Thus, any call to Instance() will auto-magically result in a call to
//      Instance() before main(), unless already present.
//  Furthermore, since the Instance() function contains the object, instead
//  of the ut::Singleton class containing a static instance of the
//  object, that object is guaranteed to be constructed (at the latest) in
//  the first call to Instance().  This permits calls to Instance() from
//  static code, even if that code is called before the file-scope objects
//  in this file have been initialized.

template <typename T>
class Singleton : public NonCopyable
{
private:
	struct Creator
	{
		// This constructor does nothing more than ensure that Instance()
		//  is called before main() begins, thus creating the static
		//  T object before multithreading race issues can come up.
		Creator() { Singleton<T>::Instance(); }
		inline void DoNothing() const { }
	};
	static Creator create_object;

	Singleton();

public:
	typedef T ObjectType;

	// If, at any point (in user code), ut::Singleton<T>::Instance()
	//  is called, then the following function is instantiated.
	static ObjectType& Instance()
	{
		// This is the object that we return a reference to.
		// It is guaranteed to be created before main() begins because of
		//  the next line.
		static ObjectType instance;

		// The following line does nothing else than force the instantiation
		//  of ut::Singleton<T>::create_object, whose constructor is
		//  called before main() begins.
		create_object.DoNothing();

		return instance;
	}
};
template <typename T>
typename Singleton<T>::Creator Singleton<T>::create_object;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
