Backward-cpp [![badge](https://img.shields.io/badge/conan.io-backward%2F0.0.0-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/source/backward/0.0.0/Manu343726/testing) [![Build Status](https://travis-ci.org/bombela/backward-cpp.svg?branch=master)](https://travis-ci.org/cwbombela/backward-cpp)
============

Backward is a beautiful stack trace pretty printer for C++.

If you are bored to see this:

![default trace](doc/rude.png)

Backward will spice it up for you:

![pretty stackstrace](doc/pretty.png)

There is not much to say. Of course it will be able to display the code
snippets only if the source files are accessible (else see trace #4 in the
example).

All "Source" lines and code snippet prefixed by a pipe "|" are frames inline
the next frame.
You can see that for the trace #1 in the example, the function
`you_shall_not_pass()` was inlined in the function `...read2::do_test()` by the
compiler.

##Installation

#### Install backward.hpp

Backward is a header only library. So installing Backward is easy, simply drop
a copy of `backward.hpp` along with your other source files in your C++ project.
You can also use a git submodule or really any other way that best fits your
environment, as long as you can include `backward.hpp`.

#### Install backward.cpp

If you want Backward to automatically print a stack trace on most common fatal
errors (segfault, abort, un-handled exception...), simply add a copy of
`backward.cpp` to your project, and don't forget to tell your build system.

The code in `backward.cpp` is trivial anyway, you can simply copy what it's
doing at your convenience.

##Configuration & Dependencies

### Integration with CMake

If you are using CMake and want to use its configuration abilities to save
you the trouble, you can easily integrate Backward, depending on how you obtained
the library.

#### As a subdirectory:

In this case you have a subdirectory containing the whole repository of Backward
(eg.: using git-submodules), in this case you can do:

```
add_subdirectory(/path/to/backward-cpp)

# This will add backward.cpp to your target
add_executable(mytarget mysource.cpp ${BACKWARD_ENABLE})

# This will add libraries, definitions and include directories needed by backward
# by setting each property on the target.
add_backward(mytarget)
```

#### Modifying CMAKE_MODULE_PATH

In this case you can have Backward installed as a subdirectory:

```
list(APPEND CMAKE_MODULE_PATH /path/to/backward-cpp)
find_package(Backward)

# This will add libraries, definitions and include directories needed by backward
# through an IMPORTED target.
target_link_libraries(mytarget PUBLIC Backward::Backward)
```

Notice that this is equivalent to using the the approach that uses `add_subdirectory()`,
however it uses cmake's [imported target](https://cmake.org/Wiki/CMake/Tutorials/Exporting_and_Importing_Targets) mechanism.

#### Installation through a regular package manager

In this case you have obtained Backward through a package manager.

Packages currently available:
- [conda-forge](https://anaconda.org/conda-forge/backward-cpp)

```
find_package(Backward)

# This will add libraries, definitions and include directories needed by backward
# through an IMPORTED target.
target_link_libraries(mytarget PUBLIC Backward::Backward)
```

### Compile with debug info

You need to compile your project with generation of debug symbols enabled,
usually `-g` with clang++ and g++.

Note that you can use `-g` with any level of optimization, with modern debug
information encoding like DWARF, it only takes space in the binary (it's not
loaded in memory until your debugger or Backward makes use of it, don't worry),
and it doesn't impact the code generation (at least on GNU/Linux x86\_64 for
what I know).

If you are missing debug information, the stack trace will lack details about
your sources.

### Libraries to read the debug info

Backward support pretty printed stack traces on GNU/Linux only, it will compile
fine under other platforms but will not do anything.  **Pull requests are
welcome :)**

Also, by default you will get a really basic stack trace, based on the
`backtrace_symbols` API:

![default trace](doc/nice.png)

You will need to install some dependencies to get the ultimate stack trace. Two
libraries are currently supported, the only difference is which one is the
easiest for you to install, so pick your poison:

#### libbfd from the [GNU/binutils](http://www.gnu.org/software/binutils/)

	apt-get install binutils-dev (or equivalent)

And do not forget to link with the lib: `g++/clang++ -lbfd ...`

Then define the following before every inclusion of `backward.hpp` (don't
forget to update `backward.cpp` as well):

	#define BACKWARD_HAS_BFD 1

#### libdw from the [elfutils](https://fedorahosted.org/elfutils/)

	apt-get install libdw-dev (or equivalent)

And do not forget to link with the lib and inform Backward to use it:

	#define BACKWARD_HAS_DW 1

Of course you can simply add the define (`-DBACKWARD_HAS_...=1`) and the
linkage details in your build system and even auto-detect which library is
installed, it's up to you.

That'ss it, you are all set, you should be getting nice stack traces like the
one at the beginning of this document.

## API

If you don't want to limit yourself to the defaults offered by `backward.cpp`,
and you want to take some random stack traces for whatever reason and pretty
print them the way you love or you decide to send them all to your buddies over
the Internet, you will appreciate the simplicity of Backward's API.

### Stacktrace

The StackTrace class lets you take a "snapshot" of the current stack.
You can use it like this:

```c++
using namespace backward;
StackTrace st; st.load_here(32);
Printer p; p.print(st);
```

The public methods are:

```c++
class StackTrace { public:
	// Take a snapshot of the current stack, with at most "trace_cnt_max"
	// traces in it. The first trace is the most recent (ie the current
	// frame). You can also provide a trace address to load_from() assuming
	// the address is a valid stack frame (useful for signal handling traces).
	// Both function return size().
	size_t load_here(size_t trace_cnt_max)
	size_t load_from(void* address, size_t trace_cnt_max)

	// The number of traces loaded. This can be less than "trace_cnt_max".
	size_t size() const

	// A unique id for the thread in which the trace was taken. The value
	// 0 means the stack trace comes from the main thread.
	size_t thread_id() const

	// Retrieve a trace by index. 0 is the most recent trace, size()-1 is
	// the oldest one.
	Trace operator[](size_t trace_idx)
};
```

### TraceResolver

The `TraceResolver` does the heavy lifting, and intends to transform a simple
`Trace` from its address into a fully detailed `ResolvedTrace` with the
filename of the source, line numbers, inlined functions and so on.

You can use it like this:

```c++
using namespace backward;
StackTrace st; st.load_here(32);

TraceResolver tr; tr.load_stacktrace(st);
for (size_t i = 0; i < st.size(); ++i) {
	ResolvedTrace trace = tr.resolve(st[i]);
	std::cout << "#" << i
		<< " " << trace.object_filename
		<< " " << trace.object_function
		<< " [" << trace.addr << "]"
	<< std::endl;
}
```

The public methods are:

```c++
class TraceResolver { public:
	// Pre-load whatever is necessary from the stack trace.
	template <class ST>
		void load_stacktrace(ST&)

	// Resolve a trace. It takes a ResolvedTrace, because a `Trace` is
	// implicitly convertible to it.
	ResolvedTrace resolve(ResolvedTrace t)
};
```

### SnippetFactory

The SnippetFactory is a simple helper class to automatically load and cache
source files in order to extract code snippets.

```c++
class SnippetFactory { public:
	// A snippet is a list of line numbers and line contents.
	typedef std::vector<std::pair<size_t, std::string> > lines_t;

	// Return a snippet starting at line_start with up to context_size lines.
	lines_t get_snippet(const std::string& filename,
			size_t line_start, size_t context_size)

	// Return a combined snippet from two different locations and combine them.
	// context_size / 2 lines will be extracted from each location.
	lines_t get_combined_snippet(
			const std::string& filename_a, size_t line_a,
			const std::string& filename_b, size_t line_b,
			size_t context_size)

	// Tries to return a unified snippet if the two locations from the same
	// file are close enough to fit inside one context_size, else returns
	// the equivalent of get_combined_snippet().
	lines_t get_coalesced_snippet(const std::string& filename,
			size_t line_a, size_t line_b, size_t context_size)
```

### Printer

A simpler way to pretty print a stack trace to the terminal. It will
automatically resolve the traces for you:

```c++
using namespace backward;
StackTrace st; st.load_here(32);
Printer p;
p.object = true;
p.color = false;
p.address = true;
p.print(st, stderr);
```

You can set a few options:

```c++
class Printer { public:
	// Print a little snippet of code if possible.
	bool snippet = true;

	// Colorize the trace (only set a color when printing on a terminal)
	bool color = true;

	// Add the addresses of every source location to the trace.
	bool address = false;

	// Even if there is a source location, prints the object the trace comes
	// from as well.
	bool object = false;

	// Resolve and print a stack trace. It takes a C FILE* object, only because
	// it is possible to access the underalying OS-level file descriptor, which
	// is then used to determine if the output is a terminal to print in color.
	template <typename StackTrace>
		FILE* print(StackTrace& st, FILE* os = stderr)
```


### SignalHandling

A simple helper class that registers for you the most common signals and other
callbacks to segfault, hardware exception, un-handled exception etc.

`backward.cpp` simply uses it like that:

```c++
backward::SignalHandling sh;
```

Creating the object registers all the different signals and hooks. Destroying
this object doesn't do anything. It exposes only one method:

```c++
bool loaded() const // true if loaded with success
```

### Trace object

To keep the memory footprint of a loaded `StackTrace` on the low-side, there a
hierarchy of trace object, from a minimal `Trace `to a `ResolvedTrace`.

#### Simple trace

```c++
struct Trace {
	void*  addr; // address of the trace
	size_t idx;  // its index (0 == most recent)
};
```

#### Resolved trace

A `ResolvedTrace` should contains a maximum of details about the location of
the trace in the source code. Note that not all fields might be set.

```c++
struct ResolvedTrace: public Trace {

	struct SourceLoc {
		std::string function;
		std::string filename;
		size_t      line;
		size_t      col;
	};

	// In which binary object this trace is located.
	std::string                    object_filename;

	// The function in the object that contains the trace. This is not the same
	// as source.function which can be an function inlined in object_function.
	std::string                    object_function;

	// The source location of this trace. It is possible for filename to be
	// empty and for line/col to be invalid (value 0) if this information
	// couldn't be deduced, for example if there is no debug information in the
	// binary object.
	SourceLoc                      source;

	// An optional list of "inliners". All of these sources locations where
	// inlined in the source location of the trace (the attribute right above).
	// This is especially useful when you compile with optimizations turned on.
	typedef std::vector<SourceLoc> source_locs_t;
	source_locs_t                  inliners;
};
```

## Contact and copyright

François-Xavier Bourlet <bombela@gmail.com>

Copyright 2013 Google Inc. All Rights Reserved.
MIT License.

### Disclaimer

Although this project is owned by Google Inc. this is not a Google supported or
affiliated project.
