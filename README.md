# UNIVERSAL TOOLS

Universal tools (UT) is an open-source cross-platform framework that is designed to make life easier for C++ developers. Goal of the UT is to be as light as possible and to provide base functionality in the same time.

### Why UT?

The same code, that is written with UT, is both c++11 and c++03 compliant (and utilizes features such as r-value and move-semantics for c++11 version). UT doesn't use stl or boost, but provides own version of containers, smart-pointers, streams, serialization system etc. Code of the UT aims to be simple, understandable and densely commented. This project is an attempt to make light universal basis for projects with no external dependencies, so that the all code could be compiled in one environment "in one click".

### Get started

UT heavily relies on [Premake utility](https://github.com/premake/premake-core) for generating project files for toolsets (like Visual Studio or GNU Make). UT has intermediate lua code to simplify generation of the projects. Visit [Premake Wiki](https://github.com/premake/premake-core/wiki) to find out how premake works.

1. Download or clone repository.
2. Create your own projects in premake.lua script. UT provides some sample projects (such as 'compatibility_test' and 'commie'), you can take one of them as a basis. Or you can leave premake.lua "as is" to figure out how it works.
3. Run appropriate script from 'build/' folder to generate project files (for example - vs2015.bat for Visual Studio 2015, or gmake.sh for GNU Make).
4. Now you can build UT workspace using project files generated in previous step.

### User Interface

[FLTK](https://www.fltk.org/) toolkit was picked for UI as it's the most light and easy-to-build UI library that was found around. Code of the fltk library was ported from CMake to Premake and was included to the UT framework (see 'contrib/fltk/' folder). Use 'bindfltk = true' option while creating utApplication to add 'fltk' project to workspace and bind it to the application project.

### Core Features

Some of the noteworthy concepts implemented in the UT library:
* Smart pointers: UniquePtr, SharedPtr, WeakPtr.
* Containers: Array, Map, Tree, AVL Tree.
* Multithreading: Thread, Job, Mutex, Synchronized, RWLock, ScopeLock.
* Non-blocking native console: input and output are asynchronous (input string is always on the bottom line).
* Serialization system: entities can be serialized in binary or text (json and xml formats are supported) form.
* Modern error handling: ut::Result (similar to std::Expected) and ut::Optional (same as std::Optional).
* Move semantics: ut::Move.
* Signal and slots: ut::Signal (similar to 'signal2' library of boost).
* Network: ut::Socket and ut::Client/ut::Server for client-server model (still is experimental).
* Encryption: Sha2, Bpkdf2, HMac, AES128.

### Contribute

Any kind of contribution is highly appreciated! If you want to become a contributor, please contact me by e-mail <universal_tools@protonmail.com>

### License

Universal tools framework is distributed under the terms of the MIT License, see LICENSE file.
In shorts - you can do whatever you want as long as you include the original copyright and license notice in any copy of the software/source.