/*
 * backward.hpp
 * Copyright 2013 Google Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef H_6B9572DA_A64B_49E6_B234_051480991C89
#define H_6B9572DA_A64B_49E6_B234_051480991C89

#ifndef __cplusplus
#	error "It's not going to compile without a C++ compiler..."
#endif

#if	  defined(BACKWARD_CXX11)
#elif defined(BACKWARD_CXX98)
#else
#	if __cplusplus >= 201103L
#		define BACKWARD_CXX11
#		define BACKWARD_ATLEAST_CXX11
#		define BACKWARD_ATLEAST_CXX98
#	else
#		define BACKWARD_CXX98
#		define BACKWARD_ATLEAST_CXX98
#	endif
#endif

// You can define one of the following (or leave it to the auto-detection):
//
// #define BACKWARD_SYSTEM_LINUX
//	- specialization for linux
//
// #define BACKWARD_SYSTEM_UNKNOWN
//	- placebo implementation, does nothing.
//
#if   defined(BACKWARD_SYSTEM_LINUX)
#elif defined(BACKWARD_SYSTEM_UNKNOWN)
#else
#	if defined(__linux)
#		define BACKWARD_SYSTEM_LINUX
#	else
#		define BACKWARD_SYSTEM_UNKNOWN
#	endif
#endif

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <new>
#include <iomanip>
#include <vector>

#if defined(BACKWARD_SYSTEM_LINUX)

// On linux, backtrace can back-trace or "walk" the stack using the following
// libraries:
//
// #define BACKWARD_HAS_UNWIND 1
//  - unwind comes from libgcc, but I saw an equivalent inside clang itself.
//  - with unwind, the stacktrace is as accurate as it can possibly be, since
//  this is used by the C++ runtine in gcc/clang for stack unwinding on
//  exception.
//  - normally libgcc is already linked to your program by default.
//
// #define BACKWARD_HAS_BACKTRACE == 1
//  - backtrace seems to be a little bit more portable than libunwind, but on
//  linux, it uses unwind anyway, but abstract away a tiny information that is
//  sadly really important in order to get perfectly accurate stack traces.
//  - backtrace is part of the (e)glib library.
//
// The default is:
// #define BACKWARD_HAS_UNWIND == 1
//
// Note that only one of the define should be set to 1 at a time.
//
#	if   BACKWARD_HAS_UNWIND == 1
#	elif BACKWARD_HAS_BACKTRACE == 1
#	else
#		undef  BACKWARD_HAS_UNWIND
#		define BACKWARD_HAS_UNWIND 1
#		undef  BACKWARD_HAS_BACKTRACE
#		define BACKWARD_HAS_BACKTRACE 0
#	endif

// On linux, backward can extract detailed information about a stack trace
// using one of the following libraries:
//
// #define BACKWARD_HAS_DW 1
//  - libdw gives you the most juicy details out of your stack traces:
//    - object filename
//    - function name
//    - source filename
//	  - line and column numbers
//	  - source code snippet (assuming the file is accessible)
//	  - variables name and values (if not optimized out)
//  - You need to link with the lib "dw":
//    - apt-get install libdw-dev
//    - g++/clang++ -ldw ...
//
// #define BACKWARD_HAS_BFD 1
//  - With libbfd, you get a fair amount of details:
//    - object filename
//    - function name
//    - source filename
//	  - line numbers
//	  - source code snippet (assuming the file is accessible)
//  - You need to link with the lib "bfd":
//    - apt-get install binutils-dev
//    - g++/clang++ -lbfd ...
//
// #define BACKWARD_HAS_BACKTRACE_SYMBOL 1
//  - backtrace provides minimal details for a stack trace:
//    - object filename
//    - function name
//  - backtrace is part of the (e)glib library.
//
// The default is:
// #define BACKWARD_HAS_BACKTRACE_SYMBOL == 1
//
// Note that only one of the define should be set to 1 at a time.
//
#	if   BACKWARD_HAS_DW == 1
#	elif BACKWARD_HAS_BFD == 1
#	elif BACKWARD_HAS_BACKTRACE_SYMBOL == 1
#	else
#		undef  BACKWARD_HAS_DW
#		define BACKWARD_HAS_DW 0
#		undef  BACKWARD_HAS_BFD
#		define BACKWARD_HAS_BFD 0
#		undef  BACKWARD_HAS_BACKTRACE_SYMBOL
#		define BACKWARD_HAS_BACKTRACE_SYMBOL 1
#	endif


#	if BACKWARD_HAS_UNWIND == 1

#		include <unwind.h>
// while gcc's unwind.h defines something like that:
//  extern _Unwind_Ptr _Unwind_GetIP (struct _Unwind_Context *);
//  extern _Unwind_Ptr _Unwind_GetIPInfo (struct _Unwind_Context *, int *);
//
// clang's unwind.h defines something like this:
//  uintptr_t _Unwind_GetIP(struct _Unwind_Context* __context);
//
// Even if the _Unwind_GetIPInfo can be linked to, it is not declared, worse we
// cannot just redeclare it because clang's unwind.h doesn't define _Unwind_Ptr
// anyway.
//
// Luckily we can play on the fact that the guard macros have a different name:
#ifdef __CLANG_UNWIND_H
// In fact, this function still comes from libgcc (on my different linux boxes,
// clang links against libgcc).
#		include <inttypes.h>
extern "C" uintptr_t _Unwind_GetIPInfo(_Unwind_Context*, int*);
#endif

#	endif

#	include <cxxabi.h>
#	include <fcntl.h>
#	include <link.h>
#	include <sys/stat.h>
#	include <syscall.h>
#	include <unistd.h>
#	include <signal.h>

#	if BACKWARD_HAS_BFD == 1
//              NOTE: defining PACKAGE{,_VERSION} is required before including
//                    bfd.h on some platforms, see also:
//                    https://sourceware.org/bugzilla/show_bug.cgi?id=14243
#               ifndef PACKAGE
#                       define PACKAGE
#               endif
#               ifndef PACKAGE_VERSION
#                       define PACKAGE_VERSION
#               endif
#		include <bfd.h>
#		ifndef _GNU_SOURCE
#			define _GNU_SOURCE
#			include <dlfcn.h>
#			undef _GNU_SOURCE
#		else
#			include <dlfcn.h>
#		endif
#	endif

#	if BACKWARD_HAS_DW == 1
#		include <elfutils/libdw.h>
#		include <elfutils/libdwfl.h>
#		include <dwarf.h>
#	endif

#	if (BACKWARD_HAS_BACKTRACE == 1) || (BACKWARD_HAS_BACKTRACE_SYMBOL == 1)
		// then we shall rely on backtrace
#		include <execinfo.h>
#	endif

#endif // defined(BACKWARD_SYSTEM_LINUX)

#ifdef BACKWARD_ATLEAST_CXX11
#	include <unordered_map>
#	include <utility> // for std::swap
	namespace backward {
	namespace details {
		template <typename K, typename V>
		struct hashtable {
			typedef std::unordered_map<K, V> type;
		};
		using std::move;
	} // namespace details
	} // namespace backward
#else // NOT BACKWARD_ATLEAST_CXX11
#	include <map>
	namespace backward {
	namespace details {
		template <typename K, typename V>
		struct hashtable {
			typedef std::map<K, V> type;
		};
		template <typename T>
			const T& move(const T& v) { return v; }
		template <typename T>
			T& move(T& v) { return v; }
	} // namespace details
	} // namespace backward
#endif // BACKWARD_ATLEAST_CXX11

namespace backward {

namespace system_tag {
	struct linux_tag; // seems that I cannot call that "linux" because the name
	// is already defined... so I am adding _tag everywhere.
	struct unknown_tag;

#if   defined(BACKWARD_SYSTEM_LINUX)
	typedef linux_tag current_tag;
#elif defined(BACKWARD_SYSTEM_UNKNOWN)
	typedef unknown_tag current_tag;
#else
#	error "May I please get my system defines?"
#endif
} // namespace system_tag


namespace trace_resolver_tag {
#ifdef BACKWARD_SYSTEM_LINUX
	struct libdw;
	struct libbfd;
	struct backtrace_symbol;

#	if   BACKWARD_HAS_DW == 1
	typedef libdw current;
#	elif BACKWARD_HAS_BFD == 1
	typedef libbfd current;
#	elif BACKWARD_HAS_BACKTRACE_SYMBOL == 1
	typedef backtrace_symbol current;
#	else
#		error "You shall not pass, until you know what you want."
#	endif
#endif // BACKWARD_SYSTEM_LINUX
} // namespace trace_resolver_tag


namespace details {

template <typename T>
	struct rm_ptr { typedef T type; };

template <typename T>
	struct rm_ptr<T*> { typedef T type; };

template <typename T>
	struct rm_ptr<const T*> { typedef const T type; };

template <typename R, typename T, R (*F)(T)>
struct deleter {
	template <typename U>
		void operator()(U& ptr) const {
			(*F)(ptr);
		}
};

template <typename T>
struct default_delete {
	void operator()(T& ptr) const {
		delete ptr;
	}
};

template <typename T, typename Deleter = deleter<void, void*, &::free> >
class handle {
	struct dummy;
	T    _val;
	bool _empty;

#ifdef BACKWARD_ATLEAST_CXX11
	handle(const handle&) = delete;
	handle& operator=(const handle&) = delete;
#endif

public:
	~handle() {
		if (!_empty) {
			Deleter()(_val);
		}
	}

	explicit handle(): _val(), _empty(true) {}
	explicit handle(T val): _val(val), _empty(false) {}

#ifdef BACKWARD_ATLEAST_CXX11
	handle(handle&& from): _empty(true) {
		swap(from);
	}
	handle& operator=(handle&& from) {
		swap(from); return *this;
	}
#else
	explicit handle(const handle& from): _empty(true) {
		// some sort of poor man's move semantic.
		swap(const_cast<handle&>(from));
	}
	handle& operator=(const handle& from) {
		// some sort of poor man's move semantic.
		swap(const_cast<handle&>(from)); return *this;
	}
#endif

	void reset(T new_val) {
		handle tmp(new_val);
		swap(tmp);
	}
	operator const dummy*() const {
		if (_empty) {
			return 0;
		}
		return reinterpret_cast<const dummy*>(_val);
	}
	T get() {
		return _val;
	}
	T release() {
		_empty = true;
		return _val;
	}
	void swap(handle& b) {
		using std::swap;
		swap(b._val, _val); // can throw, we are safe here.
		swap(b._empty, _empty); // should not throw: if you cannot swap two
		// bools without throwing... It's a lost cause anyway!
	}

	T operator->() { return _val; }
	const T operator->() const { return _val; }

	typedef typename rm_ptr<T>::type& ref_t;
	typedef const typename rm_ptr<T>::type& const_ref_t;
	ref_t operator*() { return *_val; }
	const_ref_t operator*() const { return *_val; }
	ref_t operator[](size_t idx) { return _val[idx]; }

	// Watch out, we've got a badass over here
	T* operator&() {
		_empty = false;
		return &_val;
	}
};

// Default demangler implementation (do nothing).
template <typename TAG>
struct demangler_impl {
	static std::string demangle(const char* funcname) {
		return funcname;
	}
};

#ifdef BACKWARD_SYSTEM_LINUX

template <>
struct demangler_impl<system_tag::current_tag> {
	demangler_impl(): _demangle_buffer_length(0) {}

	std::string demangle(const char* funcname) {
		using namespace details;
		_demangle_buffer.reset(
				abi::__cxa_demangle(funcname, _demangle_buffer.release(),
					&_demangle_buffer_length, 0)
				);
		if (_demangle_buffer) {
			return _demangle_buffer.get();
		}
		return funcname;
	}

private:
	details::handle<char*> _demangle_buffer;
	size_t                 _demangle_buffer_length;
};

#endif // BACKWARD_SYSTEM_LINUX

struct demangler:
	public demangler_impl<system_tag::current_tag> {};

} // namespace details

/*************** A TRACE ***************/

struct Trace {
	void*    addr;
	unsigned idx;

	Trace():
		addr(0), idx(0) {}

	explicit Trace(void* addr, size_t idx):
		addr(addr), idx(idx) {}
};

struct ResolvedTrace: public Trace {

	struct SourceLoc {
		std::string function;
		std::string filename;
		unsigned    line;
		unsigned    col;

		SourceLoc(): line(0), col(0) {}

		bool operator==(const SourceLoc& b) const {
			return function == b.function
				&& filename == b.filename
				&& line == b.line
				&& col == b.col;
		}

		bool operator!=(const SourceLoc& b) const {
			return !(*this == b);
		}
	};

	// In which binary object this trace is located.
	std::string                    object_filename;

	// The function in the object that contain the trace. This is not the same
	// as source.function which can be an function inlined in object_function.
	std::string                    object_function;

	// The source location of this trace. It is possible for filename to be
	// empty and for line/col to be invalid (value 0) if this information
	// couldn't be deduced, for example if there is no debug information in the
	// binary object.
	SourceLoc                      source;

	// An optionals list of "inliners". All the successive sources location
	// from where the source location of the trace (the attribute right above)
	// is inlined. It is especially useful when you compiled with optimization.
	typedef std::vector<SourceLoc> source_locs_t;
	source_locs_t                  inliners;

	ResolvedTrace():
		Trace() {}
	ResolvedTrace(const Trace& mini_trace):
		Trace(mini_trace) {}
};

/*************** STACK TRACE ***************/

// default implemention.
template <typename TAG>
class StackTraceImpl {
public:
	size_t size() const { return 0; }
	Trace operator[](size_t) { return Trace(); }
	size_t load_here(size_t=0) { return 0; }
	size_t load_from(void*, size_t=0) { return 0; }
	unsigned thread_id() const { return 0; }
	void skip_n_firsts(size_t) { }
};

#ifdef BACKWARD_SYSTEM_LINUX

class StackTraceLinuxImplBase {
public:
	StackTraceLinuxImplBase(): _thread_id(0), _skip(0) {}

	unsigned thread_id() const {
		return _thread_id;
	}

	void skip_n_firsts(size_t n) { _skip = n; }

protected:
	void load_thread_info() {
		_thread_id = syscall(SYS_gettid);
		if (_thread_id == (size_t) getpid()) {
			// If the thread is the main one, let's hide that.
			// I like to keep little secret sometimes.
			_thread_id = 0;
		}
	}

	size_t skip_n_firsts() const { return _skip; }

private:
	size_t _thread_id;
	size_t _skip;
};

class StackTraceLinuxImplHolder: public StackTraceLinuxImplBase {
public:
	size_t size() const {
		return _stacktrace.size() ? _stacktrace.size() - skip_n_firsts() : 0;
	}
	Trace operator[](size_t idx) {
		if (idx >= size()) {
			return Trace();
		}
		return Trace(_stacktrace[idx + skip_n_firsts()], idx);
	}
	void** begin() {
		if (size()) {
			return &_stacktrace[skip_n_firsts()];
		}
		return 0;
	}

protected:
	std::vector<void*> _stacktrace;
};


#if BACKWARD_HAS_UNWIND == 1

namespace details {

template <typename F>
class Unwinder {
public:
	size_t operator()(F& f, size_t depth) {
		_f = &f;
		_index = -1;
		_depth = depth;
		_Unwind_Backtrace(&this->backtrace_trampoline, this);
		return _index;
	}

private:
	F*      _f;
	ssize_t _index;
	size_t  _depth;

	static _Unwind_Reason_Code backtrace_trampoline(
			_Unwind_Context* ctx, void *self) {
		return ((Unwinder*)self)->backtrace(ctx);
	}

	_Unwind_Reason_Code backtrace(_Unwind_Context* ctx) {
		if (_index >= 0 && static_cast<size_t>(_index) >= _depth)
			return _URC_END_OF_STACK;

		int ip_before_instruction = 0;
		uintptr_t ip = _Unwind_GetIPInfo(ctx, &ip_before_instruction);

		if (!ip_before_instruction) {
			ip -= 1;
		}

		if (_index >= 0) { // ignore first frame.
			(*_f)(_index, (void*)ip);
		}
		_index += 1;
		return _URC_NO_REASON;
	}
};

template <typename F>
size_t unwind(F f, size_t depth) {
	Unwinder<F> unwinder;
	return unwinder(f, depth);
}

} // namespace details


template <>
class StackTraceImpl<system_tag::linux_tag>: public StackTraceLinuxImplHolder {
public:
	__attribute__ ((noinline)) // TODO use some macro
	size_t load_here(size_t depth=32) {
		load_thread_info();
		if (depth == 0) {
			return 0;
		}
		_stacktrace.resize(depth);
		size_t trace_cnt = details::unwind(callback(*this), depth);
		_stacktrace.resize(trace_cnt);
		skip_n_firsts(0);
		return size();
	}
	size_t load_from(void* addr, size_t depth=32) {
		load_here(depth + 8);

		for (size_t i = 0; i < _stacktrace.size(); ++i) {
			if (_stacktrace[i] == addr) {
				skip_n_firsts(i);
				break;
			}
		}

		_stacktrace.resize(std::min(_stacktrace.size(),
					skip_n_firsts() + depth));
		return size();
	}

private:
	struct callback {
		StackTraceImpl& self;
		callback(StackTraceImpl& self): self(self) {}

		void operator()(size_t idx, void* addr) {
			self._stacktrace[idx] = addr;
		}
	};
};


#else // BACKWARD_HAS_UNWIND == 0

template <>
class StackTraceImpl<system_tag::linux_tag>: public StackTraceLinuxImplHolder {
public:
	__attribute__ ((noinline)) // TODO use some macro
	size_t load_here(size_t depth=32) {
		load_thread_info();
		if (depth == 0) {
			return 0;
		}
		_stacktrace.resize(depth + 1);
		size_t trace_cnt = backtrace(&_stacktrace[0], _stacktrace.size());
		_stacktrace.resize(trace_cnt);
		skip_n_firsts(1);
		return size();
	}

	size_t load_from(void* addr, size_t depth=32) {
		load_here(depth + 8);

		for (size_t i = 0; i < _stacktrace.size(); ++i) {
			if (_stacktrace[i] == addr) {
				skip_n_firsts(i);
				_stacktrace[i] = (void*)( (uintptr_t)_stacktrace[i] + 1);
				break;
			}
		}

		_stacktrace.resize(std::min(_stacktrace.size(),
					skip_n_firsts() + depth));
		return size();
	}
};

#endif // BACKWARD_HAS_UNWIND
#endif // BACKWARD_SYSTEM_LINUX

class StackTrace:
	public StackTraceImpl<system_tag::current_tag> {};

/*************** TRACE RESOLVER ***************/

template <typename TAG>
class TraceResolverImpl;

#ifdef BACKWARD_SYSTEM_UNKNOWN

template <>
class TraceResolverImpl<system_tag::unknown_tag> {
public:
	template <class ST>
		void load_stacktrace(ST&) {}
	ResolvedTrace resolve(ResolvedTrace t) {
		return t;
	}
};

#endif

#ifdef BACKWARD_SYSTEM_LINUX

class TraceResolverLinuxImplBase {
protected:
	std::string demangle(const char* funcname) {
		return _demangler.demangle(funcname);
	}

private:
	details::demangler _demangler;
};

template <typename STACKTRACE_TAG>
class TraceResolverLinuxImpl;

#if BACKWARD_HAS_BACKTRACE_SYMBOL == 1

template <>
class TraceResolverLinuxImpl<trace_resolver_tag::backtrace_symbol>:
	public TraceResolverLinuxImplBase {
public:
	template <class ST>
		void load_stacktrace(ST& st) {
			using namespace details;
			if (st.size() == 0) {
				return;
			}
			_symbols.reset(
					backtrace_symbols(st.begin(), st.size())
					);
		}

	ResolvedTrace resolve(ResolvedTrace trace) {
		char* filename = _symbols[trace.idx];
		char* funcname = filename;
		while (*funcname && *funcname != '(') {
			funcname += 1;
		}
		trace.object_filename.assign(filename, funcname++);
		char* funcname_end = funcname;
		while (*funcname_end && *funcname_end != ')' && *funcname_end != '+') {
			funcname_end += 1;
		}
		*funcname_end = '\0';
		trace.object_function = this->demangle(funcname);
		trace.source.function = trace.object_function; // we cannot do better.
		return trace;
	}

private:
	details::handle<char**> _symbols;
};

#endif // BACKWARD_HAS_BACKTRACE_SYMBOL == 1

#if BACKWARD_HAS_BFD == 1

template <>
class TraceResolverLinuxImpl<trace_resolver_tag::libbfd>:
	public TraceResolverLinuxImplBase {
public:
	TraceResolverLinuxImpl(): _bfd_loaded(false) {}

	template <class ST>
		void load_stacktrace(ST&) {}

	ResolvedTrace resolve(ResolvedTrace trace) {
		Dl_info symbol_info;

		// trace.addr is a virtual address in memory pointing to some code.
		// Let's try to find from which loaded object it comes from.
		// The loaded object can be yourself btw.
		if (!dladdr(trace.addr, &symbol_info)) {
			return trace; // dat broken trace...
		}

		// Now we get in symbol_info:
		// .dli_fname:
		//		pathname of the shared object that contains the address.
		// .dli_fbase:
		//		where the object is loaded in memory.
		// .dli_sname:
		//		the name of the nearest symbol to trace.addr, we expect a
		//		function name.
		// .dli_saddr:
		//		the exact address corresponding to .dli_sname.

		if (symbol_info.dli_sname) {
			trace.object_function = demangle(symbol_info.dli_sname);
		}

		if (!symbol_info.dli_fname) {
			return trace;
		}

		trace.object_filename = symbol_info.dli_fname;
		bfd_fileobject& fobj = load_object_with_bfd(symbol_info.dli_fname);
		if (!fobj.handle) {
			return trace; // sad, we couldn't load the object :(
		}


		find_sym_result* details_selected; // to be filled.

		// trace.addr is the next instruction to be executed after returning
		// from the nested stack frame. In C++ this usually relate to the next
		// statement right after the function call that leaded to a new stack
		// frame. This is not usually what you want to see when printing out a
		// stacktrace...
		find_sym_result details_call_site = find_symbol_details(fobj,
				trace.addr, symbol_info.dli_fbase);
		details_selected = &details_call_site;

#if BACKWARD_HAS_UNWIND == 0
		// ...this is why we also try to resolve the symbol that is right
		// before the return address. If we are lucky enough, we will get the
		// line of the function that was called. But if the code is optimized,
		// we might get something absolutely not related since the compiler
		// can reschedule the return address with inline functions and
		// tail-call optimisation (among other things that I don't even know
		// or cannot even dream about with my tiny limited brain).
		find_sym_result details_adjusted_call_site = find_symbol_details(fobj,
				(void*) (uintptr_t(trace.addr) - 1),
				symbol_info.dli_fbase);

		// In debug mode, we should always get the right thing(TM).
		if (details_call_site.found && details_adjusted_call_site.found) {
			// Ok, we assume that details_adjusted_call_site is a better estimation.
			details_selected = &details_adjusted_call_site;
			trace.addr = (void*) (uintptr_t(trace.addr) - 1);
		}

		if (details_selected == &details_call_site && details_call_site.found) {
			// we have to re-resolve the symbol in order to reset some
			// internal state in BFD... so we can call backtrace_inliners
			// thereafter...
			details_call_site = find_symbol_details(fobj, trace.addr,
					symbol_info.dli_fbase);
		}
#endif // BACKWARD_HAS_UNWIND

		if (details_selected->found) {
			if (details_selected->filename) {
				trace.source.filename = details_selected->filename;
			}
			trace.source.line = details_selected->line;

			if (details_selected->funcname) {
				// this time we get the name of the function where the code is
				// located, instead of the function were the address is
				// located. In short, if the code was inlined, we get the
				// function correspoding to the code. Else we already got in
				// trace.function.
				trace.source.function = demangle(details_selected->funcname);

				if (!symbol_info.dli_sname) {
					// for the case dladdr failed to find the symbol name of
					// the function, we might as well try to put something
					// here.
					trace.object_function = trace.source.function;
				}
			}

			// Maybe the source of the trace got inlined inside the function
			// (trace.source.function). Let's see if we can get all the inlined
			// calls along the way up to the initial call site.
			trace.inliners = backtrace_inliners(fobj, *details_selected);

#if 0
			if (trace.inliners.size() == 0) {
				// Maybe the trace was not inlined... or maybe it was and we
				// are lacking the debug information. Let's try to make the
				// world better and see if we can get the line number of the
				// function (trace.source.function) now.
				//
				// We will get the location of where the function start (to be
				// exact: the first instruction that really start the
				// function), not where the name of the function is defined.
				// This can be quite far away from the name of the function
				// btw.
				//
				// If the source of the function is the same as the source of
				// the trace, we cannot say if the trace was really inlined or
				// not.  However, if the filename of the source is different
				// between the function and the trace... we can declare it as
				// an inliner.  This is not 100% accurate, but better than
				// nothing.

				if (symbol_info.dli_saddr) {
					find_sym_result details = find_symbol_details(fobj,
							symbol_info.dli_saddr,
							symbol_info.dli_fbase);

					if (details.found) {
						ResolvedTrace::SourceLoc diy_inliner;
						diy_inliner.line = details.line;
						if (details.filename) {
							diy_inliner.filename = details.filename;
						}
						if (details.funcname) {
							diy_inliner.function = demangle(details.funcname);
						} else {
							diy_inliner.function = trace.source.function;
						}
						if (diy_inliner != trace.source) {
							trace.inliners.push_back(diy_inliner);
						}
					}
				}
			}
#endif
		}

		return trace;
	}

private:
	bool                _bfd_loaded;

	typedef details::handle<bfd*,
			details::deleter<bfd_boolean, bfd*, &bfd_close>
				> bfd_handle_t;

	typedef details::handle<asymbol**> bfd_symtab_t;


	struct bfd_fileobject {
		bfd_handle_t handle;
		bfd_vma      base_addr;
		bfd_symtab_t symtab;
		bfd_symtab_t dynamic_symtab;
	};

	typedef details::hashtable<std::string, bfd_fileobject>::type
		fobj_bfd_map_t;
	fobj_bfd_map_t      _fobj_bfd_map;

	bfd_fileobject& load_object_with_bfd(const std::string& filename_object) {
		using namespace details;

		if (!_bfd_loaded) {
			using namespace details;
			bfd_init();
			_bfd_loaded = true;
		}

		fobj_bfd_map_t::iterator it =
			_fobj_bfd_map.find(filename_object);
		if (it != _fobj_bfd_map.end()) {
			return it->second;
		}

		// this new object is empty for now.
		bfd_fileobject& r = _fobj_bfd_map[filename_object];

		// we do the work temporary in this one;
		bfd_handle_t bfd_handle;

		int fd = open(filename_object.c_str(), O_RDONLY);
		bfd_handle.reset(
				bfd_fdopenr(filename_object.c_str(), "default", fd)
				);
		if (!bfd_handle) {
			close(fd);
			return r;
		}

		if (!bfd_check_format(bfd_handle.get(), bfd_object)) {
			return r; // not an object? You lose.
		}

		if ((bfd_get_file_flags(bfd_handle.get()) & HAS_SYMS) == 0) {
			return r; // that's what happen when you forget to compile in debug.
		}

		ssize_t symtab_storage_size =
			bfd_get_symtab_upper_bound(bfd_handle.get());

		ssize_t dyn_symtab_storage_size =
			bfd_get_dynamic_symtab_upper_bound(bfd_handle.get());

		if (symtab_storage_size <= 0 && dyn_symtab_storage_size <= 0) {
			return r; // weird, is the file is corrupted?
		}

		bfd_symtab_t symtab, dynamic_symtab;
		ssize_t symcount = 0, dyn_symcount = 0;

		if (symtab_storage_size > 0) {
			symtab.reset(
					(bfd_symbol**) malloc(symtab_storage_size)
					);
			symcount = bfd_canonicalize_symtab(
					bfd_handle.get(), symtab.get()
					);
		}

		if (dyn_symtab_storage_size > 0) {
			dynamic_symtab.reset(
					(bfd_symbol**) malloc(dyn_symtab_storage_size)
					);
			dyn_symcount = bfd_canonicalize_dynamic_symtab(
					bfd_handle.get(), dynamic_symtab.get()
					);
		}


		if (symcount <= 0 && dyn_symcount <= 0) {
			return r; // damned, that's a stripped file that you got there!
		}

		r.handle = move(bfd_handle);
		r.symtab = move(symtab);
		r.dynamic_symtab = move(dynamic_symtab);
		return r;
	}

	struct find_sym_result {
		bool found;
		const char* filename;
		const char* funcname;
		unsigned int line;
	};

	struct find_sym_context {
		TraceResolverLinuxImpl* self;
		bfd_fileobject* fobj;
		void* addr;
		void* base_addr;
		find_sym_result result;
	};

	find_sym_result find_symbol_details(bfd_fileobject& fobj, void* addr,
			void* base_addr) {
		find_sym_context context;
		context.self = this;
		context.fobj = &fobj;
		context.addr = addr;
		context.base_addr = base_addr;
		context.result.found = false;
		bfd_map_over_sections(fobj.handle.get(), &find_in_section_trampoline,
				(void*)&context);
		return context.result;
	}

	static void find_in_section_trampoline(bfd*, asection* section,
			void* data) {
		find_sym_context* context = static_cast<find_sym_context*>(data);
		context->self->find_in_section(
				reinterpret_cast<bfd_vma>(context->addr),
				reinterpret_cast<bfd_vma>(context->base_addr),
				*context->fobj,
				section, context->result
				);
	}

	void find_in_section(bfd_vma addr, bfd_vma base_addr,
			bfd_fileobject& fobj, asection* section, find_sym_result& result)
	{
		if (result.found) return;

		if ((bfd_get_section_flags(fobj.handle.get(), section)
					& SEC_ALLOC) == 0)
			return; // a debug section is never loaded automatically.

		bfd_vma sec_addr = bfd_get_section_vma(fobj.handle.get(), section);
		bfd_size_type size = bfd_get_section_size(section);

		// are we in the boundaries of the section?
		if (addr < sec_addr || addr >= sec_addr + size) {
			addr -= base_addr; // oups, a relocated object, lets try again...
			if (addr < sec_addr || addr >= sec_addr + size) {
				return;
			}
		}

		if (!result.found && fobj.symtab) {
			result.found = bfd_find_nearest_line(fobj.handle.get(), section,
					fobj.symtab.get(), addr - sec_addr, &result.filename,
					&result.funcname, &result.line);
		}

		if (!result.found && fobj.dynamic_symtab) {
			result.found = bfd_find_nearest_line(fobj.handle.get(), section,
					fobj.dynamic_symtab.get(), addr - sec_addr,
					&result.filename, &result.funcname, &result.line);
		}

	}

	ResolvedTrace::source_locs_t backtrace_inliners(bfd_fileobject& fobj,
			find_sym_result previous_result) {
		// This function can be called ONLY after a SUCCESSFUL call to
		// find_symbol_details. The state is global to the bfd_handle.
		ResolvedTrace::source_locs_t results;
		while (previous_result.found) {
			find_sym_result result;
			result.found = bfd_find_inliner_info(fobj.handle.get(),
					&result.filename, &result.funcname, &result.line);

			if (result.found) /* and not (
						cstrings_eq(previous_result.filename, result.filename)
						and cstrings_eq(previous_result.funcname, result.funcname)
						and result.line == previous_result.line
						)) */ {
				ResolvedTrace::SourceLoc src_loc;
				src_loc.line = result.line;
				if (result.filename) {
					src_loc.filename = result.filename;
				}
				if (result.funcname) {
					src_loc.function = demangle(result.funcname);
				}
				results.push_back(src_loc);
			}
			previous_result = result;
		}
		return results;
	}

	bool cstrings_eq(const char* a, const char* b) {
		if (!a || !b) {
			return false;
		}
		return strcmp(a, b) == 0;
	}

};
#endif // BACKWARD_HAS_BFD == 1

#if BACKWARD_HAS_DW == 1

template <>
class TraceResolverLinuxImpl<trace_resolver_tag::libdw>:
	public TraceResolverLinuxImplBase {
public:
	TraceResolverLinuxImpl(): _dwfl_handle_initialized(false) {}

	template <class ST>
		void load_stacktrace(ST&) {}

	ResolvedTrace resolve(ResolvedTrace trace) {
		using namespace details;

		Dwarf_Addr trace_addr = (Dwarf_Addr) trace.addr;

		if (!_dwfl_handle_initialized) {
			// initialize dwfl...
			_dwfl_cb.reset(new Dwfl_Callbacks);
			_dwfl_cb->find_elf = &dwfl_linux_proc_find_elf;
			_dwfl_cb->find_debuginfo = &dwfl_standard_find_debuginfo;
			_dwfl_cb->debuginfo_path = 0;

			_dwfl_handle.reset(dwfl_begin(_dwfl_cb.get()));
			_dwfl_handle_initialized = true;

			if (!_dwfl_handle) {
				return trace;
			}

			// ...from the current process.
			dwfl_report_begin(_dwfl_handle.get());
			int r = dwfl_linux_proc_report (_dwfl_handle.get(), getpid());
			dwfl_report_end(_dwfl_handle.get(), NULL, NULL);
			if (r < 0) {
				return trace;
			}
		}

		if (!_dwfl_handle) {
			return trace;
		}

		// find the module (binary object) that contains the trace's address.
		// This is not using any debug information, but the addresses ranges of
		// all the currently loaded binary object.
		Dwfl_Module* mod = dwfl_addrmodule(_dwfl_handle.get(), trace_addr);
		if (mod) {
			// now that we found it, lets get the name of it, this will be the
			// full path to the running binary or one of the loaded library.
			const char* module_name = dwfl_module_info (mod,
					0, 0, 0, 0, 0, 0, 0);
			if (module_name) {
				trace.object_filename = module_name;
			}
			// We also look after the name of the symbol, equal or before this
			// address. This is found by walking the symtab. We should get the
			// symbol corresponding to the function (mangled) containing the
			// address. If the code corresponding to the address was inlined,
			// this is the name of the out-most inliner function.
			const char* sym_name = dwfl_module_addrname(mod, trace_addr);
			if (sym_name) {
				trace.object_function = demangle(sym_name);
			}
		}

		// now let's get serious, and find out the source location (file and
		// line number) of the address.

		// This function will look in .debug_aranges for the address and map it
		// to the location of the compilation unit DIE in .debug_info and
		// return it.
		Dwarf_Addr mod_bias = 0;
		Dwarf_Die* cudie = dwfl_module_addrdie(mod, trace_addr, &mod_bias);

#if 1
		if (!cudie) {
			// Sadly clang does not generate the section .debug_aranges, thus
			// dwfl_module_addrdie will fail early. Clang doesn't either set
			// the lowpc/highpc/range info for every compilation unit.
			//
			// So in order to save the world:
			// for every compilation unit, we will iterate over every single
			// DIEs. Normally functions should have a lowpc/highpc/range, which
			// we will use to infer the compilation unit.

			// note that this is probably badly inefficient.
			while ((cudie = dwfl_module_nextcu(mod, cudie, &mod_bias))) {
				Dwarf_Die die_mem;
				Dwarf_Die* fundie = find_fundie_by_pc(cudie,
						trace_addr - mod_bias, &die_mem);
				if (fundie) {
					break;
				}
			}
		}
#endif

//#define BACKWARD_I_DO_NOT_RECOMMEND_TO_ENABLE_THIS_HORRIBLE_PIECE_OF_CODE
#ifdef BACKWARD_I_DO_NOT_RECOMMEND_TO_ENABLE_THIS_HORRIBLE_PIECE_OF_CODE
		if (!cudie) {
			// If it's still not enough, lets dive deeper in the shit, and try
			// to save the world again: for every compilation unit, we will
			// load the corresponding .debug_line section, and see if we can
			// find our address in it.

			Dwarf_Addr cfi_bias;
			Dwarf_CFI* cfi_cache = dwfl_module_eh_cfi(mod, &cfi_bias);

			Dwarf_Addr bias;
			while ((cudie = dwfl_module_nextcu(mod, cudie, &bias))) {
				if (dwarf_getsrc_die(cudie, trace_addr - bias)) {

					// ...but if we get a match, it might be a false positive
					// because our (address - bias) might as well be valid in a
					// different compilation unit. So we throw our last card on
					// the table and lookup for the address into the .eh_frame
					// section.

					handle<Dwarf_Frame*> frame;
					dwarf_cfi_addrframe(cfi_cache, trace_addr - cfi_bias, &frame);
					if (frame) {
						break;
					}
				}
			}
		}
#endif

		if (!cudie) {
			return trace; // this time we lost the game :/
		}

		// Now that we have a compilation unit DIE, this function will be able
		// to load the corresponding section in .debug_line (if not already
		// loaded) and hopefully find the source location mapped to our
		// address.
		Dwarf_Line* srcloc = dwarf_getsrc_die(cudie, trace_addr - mod_bias);

		if (srcloc) {
			const char* srcfile = dwarf_linesrc(srcloc, 0, 0);
			if (srcfile) {
				trace.source.filename = srcfile;
			}
			int line = 0, col = 0;
			dwarf_lineno(srcloc, &line);
			dwarf_linecol(srcloc, &col);
			trace.source.line = line;
			trace.source.col = col;
		}

		deep_first_search_by_pc(cudie, trace_addr - mod_bias,
				inliners_search_cb(trace));
		if (trace.source.function.size() == 0) {
			// fallback.
			trace.source.function = trace.object_function;
		}

		return trace;
	}

private:
	typedef details::handle<Dwfl*, details::deleter<void, Dwfl*, &dwfl_end> >
		dwfl_handle_t;
	details::handle<Dwfl_Callbacks*, details::default_delete<Dwfl_Callbacks*> >
		           _dwfl_cb;
	dwfl_handle_t  _dwfl_handle;
	bool           _dwfl_handle_initialized;

	// defined here because in C++98, template function cannot take locally
	// defined types... grrr.
	struct inliners_search_cb {
		void operator()(Dwarf_Die* die) {
			switch (dwarf_tag(die)) {
				const char* name;
				case DW_TAG_subprogram:
					if ((name = dwarf_diename(die))) {
						trace.source.function = name;
					}
					break;

				case DW_TAG_inlined_subroutine:
					ResolvedTrace::SourceLoc sloc;
					Dwarf_Attribute attr_mem;

					if ((name = dwarf_diename(die))) {
						sloc.function = name;
					}
					if ((name = die_call_file(die))) {
						sloc.filename = name;
					}

					Dwarf_Word line = 0, col = 0;
					dwarf_formudata(dwarf_attr(die, DW_AT_call_line,
								&attr_mem), &line);
					dwarf_formudata(dwarf_attr(die, DW_AT_call_column,
								&attr_mem), &col);
					sloc.line = line;
					sloc.col = col;

					trace.inliners.push_back(sloc);
					break;
			};
		}
		ResolvedTrace& trace;
		inliners_search_cb(ResolvedTrace& t): trace(t) {}
	};


	static bool die_has_pc(Dwarf_Die* die, Dwarf_Addr pc) {
		Dwarf_Addr low, high;

		// continuous range
		if (dwarf_hasattr(die, DW_AT_low_pc) and
							dwarf_hasattr(die, DW_AT_high_pc)) {
			if (dwarf_lowpc(die, &low) != 0) {
				return false;
			}
			if (dwarf_highpc(die, &high) != 0) {
				Dwarf_Attribute attr_mem;
				Dwarf_Attribute* attr = dwarf_attr(die, DW_AT_high_pc, &attr_mem);
				Dwarf_Word value;
				if (dwarf_formudata(attr, &value) != 0) {
					return false;
				}
				high = low + value;
			}
			return pc >= low && pc < high;
		}

		// non-continuous range.
		Dwarf_Addr base;
		ptrdiff_t offset = 0;
		while ((offset = dwarf_ranges(die, offset, &base, &low, &high)) > 0) {
			if (pc >= low && pc < high) {
				return true;
			}
		}
		return false;
	}

	static Dwarf_Die* find_fundie_by_pc(Dwarf_Die* parent_die, Dwarf_Addr pc,
			Dwarf_Die* result) {
		if (dwarf_child(parent_die, result) != 0) {
			return 0;
		}

		Dwarf_Die* die = result;
		do {
			switch (dwarf_tag(die)) {
				case DW_TAG_subprogram:
				case DW_TAG_inlined_subroutine:
					if (die_has_pc(die, pc)) {
						return result;
					}
				default:
					bool declaration = false;
					Dwarf_Attribute attr_mem;
					dwarf_formflag(dwarf_attr(die, DW_AT_declaration,
								&attr_mem), &declaration);
					if (!declaration) {
						// let's be curious and look deeper in the tree,
						// function are not necessarily at the first level, but
						// might be nested inside a namespace, structure etc.
						Dwarf_Die die_mem;
						Dwarf_Die* indie = find_fundie_by_pc(die, pc, &die_mem);
						if (indie) {
							*result = die_mem;
							return result;
						}
					}
			};
		} while (dwarf_siblingof(die, result) == 0);
		return 0;
	}

	template <typename CB>
		static bool deep_first_search_by_pc(Dwarf_Die* parent_die,
				Dwarf_Addr pc, CB cb) {
		Dwarf_Die die_mem;
		if (dwarf_child(parent_die, &die_mem) != 0) {
			return false;
		}

		bool branch_has_pc = false;
		Dwarf_Die* die = &die_mem;
		do {
			bool declaration = false;
			Dwarf_Attribute attr_mem;
			dwarf_formflag(dwarf_attr(die, DW_AT_declaration, &attr_mem), &declaration);
			if (!declaration) {
				// let's be curious and look deeper in the tree, function are
				// not necessarily at the first level, but might be nested
				// inside a namespace, structure, a function, an inlined
				// function etc.
				branch_has_pc = deep_first_search_by_pc(die, pc, cb);
			}
			if (!branch_has_pc) {
				branch_has_pc = die_has_pc(die, pc);
			}
			if (branch_has_pc) {
				cb(die);
			}
		} while (dwarf_siblingof(die, &die_mem) == 0);
		return branch_has_pc;
	}

	static const char* die_call_file(Dwarf_Die *die) {
		Dwarf_Attribute attr_mem;
		Dwarf_Sword file_idx = 0;

		dwarf_formsdata(dwarf_attr(die, DW_AT_call_file, &attr_mem),
				&file_idx);

		if (file_idx == 0) {
			return 0;
		}

		Dwarf_Die die_mem;
		Dwarf_Die* cudie = dwarf_diecu(die, &die_mem, 0, 0);
		if (!cudie) {
			return 0;
		}

		Dwarf_Files* files = 0;
		size_t nfiles;
		dwarf_getsrcfiles(cudie, &files, &nfiles);
		if (!files) {
			return 0;
		}

		return dwarf_filesrc(files, file_idx, 0, 0);
	}

};
#endif // BACKWARD_HAS_DW == 1

template<>
class TraceResolverImpl<system_tag::linux_tag>:
	public TraceResolverLinuxImpl<trace_resolver_tag::current> {};

#endif // BACKWARD_SYSTEM_LINUX

class TraceResolver:
	public TraceResolverImpl<system_tag::current_tag> {};

/*************** CODE SNIPPET ***************/

class SourceFile {
public:
	typedef std::vector<std::pair<unsigned, std::string> > lines_t;

	SourceFile() {}
	SourceFile(const std::string& path): _file(new std::ifstream(path.c_str())) {}
	bool is_open() const { return _file->is_open(); }

	lines_t& get_lines(unsigned line_start, unsigned line_count, lines_t& lines) {
		using namespace std;
		// This function make uses of the dumbest algo ever:
		//	1) seek(0)
		//	2) read lines one by one and discard until line_start
		//	3) read line one by one until line_start + line_count
		//
		// If you are getting snippets many time from the same file, it is
		// somewhat a waste of CPU, feel free to benchmark and propose a
		// better solution ;)

		_file->clear();
		_file->seekg(0);
		string line;
		unsigned line_idx;

		for (line_idx = 1; line_idx < line_start; ++line_idx) {
			std::getline(*_file, line);
			if (!*_file) {
				return lines;
			}
		}

		// think of it like a lambda in C++98 ;)
		// but look, I will reuse it two times!
		// What a good boy am I.
		struct isspace {
			bool operator()(char c) {
				return std::isspace(c);
			}
		};

		bool started = false;
		for (; line_idx < line_start + line_count; ++line_idx) {
			getline(*_file, line);
			if (!*_file) {
				return lines;
			}
			if (!started) {
				if (std::find_if(line.begin(), line.end(),
							not_isspace()) == line.end())
					continue;
				started = true;
			}
			lines.push_back(make_pair(line_idx, line));
		}

		lines.erase(
				std::find_if(lines.rbegin(), lines.rend(),
					not_isempty()).base(), lines.end()
				);
		return lines;
	}

	lines_t get_lines(unsigned line_start, unsigned line_count) {
		lines_t lines;
		return get_lines(line_start, line_count, lines);
	}

	// there is no find_if_not in C++98, lets do something crappy to
	// workaround.
	struct not_isspace {
		bool operator()(char c) {
			return !std::isspace(c);
		}
	};
	// and define this one here because C++98 is not happy with local defined
	// struct passed to template functions, fuuuu.
	struct not_isempty {
		bool operator()(const lines_t::value_type& p) {
			return !(std::find_if(p.second.begin(), p.second.end(),
						not_isspace()) == p.second.end());
		}
	};

	void swap(SourceFile& b) {
		_file.swap(b._file);
	}

#ifdef BACKWARD_ATLEAST_CXX11
	SourceFile(SourceFile&& from): _file(0) {
		swap(from);
	}
	SourceFile& operator=(SourceFile&& from) {
		swap(from); return *this;
	}
#else
	explicit SourceFile(const SourceFile& from) {
		// some sort of poor man's move semantic.
		swap(const_cast<SourceFile&>(from));
	}
	SourceFile& operator=(const SourceFile& from) {
		// some sort of poor man's move semantic.
		swap(const_cast<SourceFile&>(from)); return *this;
	}
#endif

private:
	details::handle<std::ifstream*,
		details::default_delete<std::ifstream*>
			> _file;

#ifdef BACKWARD_ATLEAST_CXX11
	SourceFile(const SourceFile&) = delete;
	SourceFile& operator=(const SourceFile&) = delete;
#endif
};

class SnippetFactory {
public:
	typedef SourceFile::lines_t lines_t;

	lines_t get_snippet(const std::string& filename,
			unsigned line_start, unsigned context_size) {

		SourceFile& src_file = get_src_file(filename);
		unsigned start = line_start - context_size / 2;
		return src_file.get_lines(start, context_size);
	}

	lines_t get_combined_snippet(
			const std::string& filename_a, unsigned line_a,
			const std::string& filename_b, unsigned line_b,
			unsigned context_size) {
		SourceFile& src_file_a = get_src_file(filename_a);
		SourceFile& src_file_b = get_src_file(filename_b);

		lines_t lines = src_file_a.get_lines(line_a - context_size / 4,
				context_size / 2);
		src_file_b.get_lines(line_b - context_size / 4, context_size / 2,
				lines);
		return lines;
	}

	lines_t get_coalesced_snippet(const std::string& filename,
			unsigned line_a, unsigned line_b, unsigned context_size) {
		SourceFile& src_file = get_src_file(filename);

		using std::min; using std::max;
		unsigned a = min(line_a, line_b);
		unsigned b = max(line_a, line_b);

		if ((b - a) < (context_size / 3)) {
			return src_file.get_lines((a + b - context_size + 1) / 2,
					context_size);
		}

		lines_t lines = src_file.get_lines(a - context_size / 4,
				context_size / 2);
		src_file.get_lines(b - context_size / 4, context_size / 2, lines);
		return lines;
	}


private:
	typedef details::hashtable<std::string, SourceFile>::type src_files_t;
	src_files_t _src_files;

	SourceFile& get_src_file(const std::string& filename) {
		src_files_t::iterator it = _src_files.find(filename);
		if (it != _src_files.end()) {
			return it->second;
		}
		SourceFile& new_src_file = _src_files[filename];
		new_src_file = SourceFile(filename);
		return new_src_file;
	}
};

/*************** PRINTER ***************/

#ifdef BACKWARD_SYSTEM_LINUX

namespace Color {
	enum type {
		yellow = 33,
		purple = 35,
		reset  = 39
	};
} // namespace Color

class Colorize {
public:
	Colorize(std::FILE* os):
		_os(os), _reset(false), _istty(false) {}

	void init() {
		_istty = isatty(fileno(_os));
	}

	void set_color(Color::type ccode) {
		if (!_istty) return;

		// I assume that the terminal can handle basic colors. Seriously I
		// don't want to deal with all the termcap shit.
		fprintf(_os, "\033[%im", static_cast<int>(ccode));
		_reset = (ccode != Color::reset);
	}

	~Colorize() {
		if (_reset) {
			set_color(Color::reset);
		}
	}

private:
	std::FILE* _os;
	bool       _reset;
	bool       _istty;
};

#else // ndef BACKWARD_SYSTEM_LINUX


namespace Color {
	enum type {
		yellow = 0,
		purple = 0,
		reset  = 0
	};
} // namespace Color

class Colorize {
public:
	Colorize(std::FILE*) {}
	void init() {}
	void set_color(Color::type) {}
};

#endif // BACKWARD_SYSTEM_LINUX

class Printer {
public:
	bool snippet;
	bool color;
	bool address;
	bool object;
	int inliner_context_size;
	int trace_context_size;

	Printer():
		snippet(true),
		color(true),
		address(false),
		object(false),
		inliner_context_size(5),
		trace_context_size(7)
		{}

	template <typename ST>
		FILE* print(ST& st, FILE* os = stderr) {
			Colorize colorize(os);
			if (color) {
				colorize.init();
			}
			print_header(os, st.thread_id());
			_resolver.load_stacktrace(st);
			for (size_t trace_idx = st.size(); trace_idx > 0; --trace_idx) {
				print_trace(os, _resolver.resolve(st[trace_idx-1]), colorize);
			}
			return os;
		}

	template <typename IT>
		FILE* print(IT begin, IT end, FILE* os = stderr, size_t thread_id = 0) {
			Colorize colorize(os);
			if (color) {
				colorize.init();
			}
			print_header(os, thread_id);
			for (; begin != end; ++begin) {
				print_trace(os, *begin, colorize);
			}
			return os;
		}
private:
	TraceResolver  _resolver;
	SnippetFactory _snippets;

	void print_header(FILE* os, unsigned thread_id) {
		fprintf(os, "Stack trace (most recent call last)");
		if (thread_id) {
			fprintf(os, " in thread %u:\n", thread_id);
		} else {
			fprintf(os, ":\n");
		}
	}

	void print_trace(FILE* os, const ResolvedTrace& trace,
			Colorize& colorize) {
		fprintf(os, "#%-2u", trace.idx);
		bool already_indented = true;

		if (!trace.source.filename.size() || object) {
			fprintf(os, "   Object \"%s\", at %p, in %s\n",
					trace.object_filename.c_str(), trace.addr,
					trace.object_function.c_str());
			already_indented = false;
		}

		for (size_t inliner_idx = trace.inliners.size();
				inliner_idx > 0; --inliner_idx) {
			if (!already_indented) {
				fprintf(os, "   ");
			}
			const ResolvedTrace::SourceLoc& inliner_loc
				= trace.inliners[inliner_idx-1];
			print_source_loc(os, " | ", inliner_loc);
			if (snippet) {
				print_snippet(os, "    | ", inliner_loc,
						colorize, Color::purple, inliner_context_size);
			}
			already_indented = false;
		}

		if (trace.source.filename.size()) {
			if (!already_indented) {
				fprintf(os, "   ");
			}
			print_source_loc(os, "   ", trace.source, trace.addr);
			if (snippet) {
				print_snippet(os, "      ", trace.source,
						colorize, Color::yellow, trace_context_size);
			}
		}
	}

	void print_snippet(FILE* os, const char* indent,
			const ResolvedTrace::SourceLoc& source_loc,
			Colorize& colorize, Color::type color_code,
			int context_size)
	{
		using namespace std;
		typedef SnippetFactory::lines_t lines_t;

		lines_t lines = _snippets.get_snippet(source_loc.filename,
				source_loc.line, context_size);

		for (lines_t::const_iterator it = lines.begin();
				it != lines.end(); ++it) {
			if (it-> first == source_loc.line) {
				colorize.set_color(color_code);
				fprintf(os, "%s>", indent);
			} else {
				fprintf(os, "%s ", indent);
			}
			fprintf(os, "%4u: %s\n", it->first, it->second.c_str());
			if (it-> first == source_loc.line) {
				colorize.set_color(Color::reset);
			}
		}
	}

	void print_source_loc(FILE* os, const char* indent,
			const ResolvedTrace::SourceLoc& source_loc,
			void* addr=0) {
		fprintf(os, "%sSource \"%s\", line %i, in %s",
				indent, source_loc.filename.c_str(), (int)source_loc.line,
				source_loc.function.c_str());

		if (address && addr != 0) {
			fprintf(os, " [%p]\n", addr);
		} else {
			fprintf(os, "\n");
		}
	}
};

/*************** SIGNALS HANDLING ***************/

#ifdef BACKWARD_SYSTEM_LINUX


class SignalHandling {
public:
   static std::vector<int> make_default_signals() {
       const int posix_signals[] = {
		// default action: Core
		SIGILL,
		SIGABRT,
		SIGFPE,
		SIGSEGV,
		SIGBUS,
		// I am not sure the following signals should be enabled by
		// default:
		// default action: Term
		SIGHUP,
		SIGINT,
		SIGPIPE,
		SIGALRM,
		SIGTERM,
		SIGUSR1,
		SIGUSR2,
		SIGPOLL,
		SIGPROF,
		SIGVTALRM,
		SIGIO,
		SIGPWR,
		// default action: Core
		SIGQUIT,
		SIGSYS,
		SIGTRAP,
		SIGXCPU,
		SIGXFSZ
	};
        return std::vector<int>(posix_signals, posix_signals + sizeof posix_signals / sizeof posix_signals[0] );
   }

  SignalHandling(const std::vector<int>& posix_signals = make_default_signals()):
	  _loaded(false) {
		bool success = true;

		const size_t stack_size = 1024 * 1024 * 8;
		_stack_content.reset((char*)malloc(stack_size));
		if (_stack_content) {
			stack_t ss;
			ss.ss_sp = _stack_content.get();
			ss.ss_size = stack_size;
			ss.ss_flags = 0;
			if (sigaltstack(&ss, 0) < 0) {
				success = false;
			}
		} else {
			success = false;
		}

		for (size_t i = 0; i < posix_signals.size(); ++i) {
			struct sigaction action;
			memset(&action, 0, sizeof action);
			action.sa_flags = (SA_SIGINFO | SA_ONSTACK | SA_NODEFER |
					SA_RESETHAND);
			sigfillset(&action.sa_mask);
			sigdelset(&action.sa_mask, posix_signals[i]);
			action.sa_sigaction = &sig_handler;

			int r = sigaction(posix_signals[i], &action, 0);
			if (r < 0) success = false;
		}

		_loaded = success;
	}

	bool loaded() const { return _loaded; }

private:
	details::handle<char*> _stack_content;
	bool                   _loaded;

	static void sig_handler(int, siginfo_t* info, void* _ctx) {
		ucontext_t *uctx = (ucontext_t*) _ctx;

		StackTrace st;
		void* error_addr = 0;
#ifdef REG_RIP // x86_64
		error_addr = reinterpret_cast<void*>(uctx->uc_mcontext.gregs[REG_RIP]);
#elif defined(REG_EIP) // x86_32
		error_addr = reinterpret_cast<void*>(uctx->uc_mcontext.gregs[REG_EIP]);
#elif defined(__arm__)
		error_addr = reinterpret_cast<void*>(uctx->uc_mcontext.arm_pc);
#elif defined(__ppc__) || defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__)
		error_addr = reinterpret_cast<void*>(uctx->uc_mcontext.regs->nip);
#else
#	warning ":/ sorry, ain't know no nothing none not of your architecture!"
#endif
		if (error_addr) {
			st.load_from(error_addr, 32);
		} else {
			st.load_here(32);
		}

		Printer printer;
		printer.address = true;
		printer.print(st, stderr);

#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
		psiginfo(info, 0);
#endif

		// try to forward the signal.
		raise(info->si_signo);

		// terminate the process immediately.
		puts("watf? exit");
		_exit(EXIT_FAILURE);
	}
};

#endif // BACKWARD_SYSTEM_LINUX

#ifdef BACKWARD_SYSTEM_UNKNOWN

class SignalHandling {
public:
	SignalHandling(const std::vector<int>& = std::vector<int>()) {}
	bool init() { return false; }
	bool loaded() { return false; }
};

#endif // BACKWARD_SYSTEM_UNKNOWN

} // namespace backward

#endif /* H_GUARD */
