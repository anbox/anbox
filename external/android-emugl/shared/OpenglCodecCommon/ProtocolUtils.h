#ifndef EMUGL_PROTOCOL_UTILS_H
#define EMUGL_PROTOCOL_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace emugl {

// Helper macro
#define COMPILE_ASSERT(cond)  static char kAssert##__LINE__[1 - 2 * !(cond)] __attribute__((unused)) = { 0 }

// Helper template: is_pointer.
// is_pointer<T>::value is true iff |T| is a pointer type.
template <typename T> struct is_pointer {
    static const bool value = false;
};

template <typename T> struct is_pointer<T*> {
    static const bool value = true;
};

// A helper template to extract values form the wire protocol stream
// and convert them to appropriate host values.
//
// The wire protocol uses 32-bit exclusively when transferring
// GLintptr or GLsizei values, as well as opaque handles like GLeglImage,
// from the guest (even when the guest is 64-bit).
//
// The corresponding host definitions depend on the host bitness. For
// example, GLintptr is 64-bit on linux-x86_64. The following is a set
// of templates that can simplify the conversion of protocol values
// into host ones.
//
// The most important one is:
//
//     unpack<HOST_TYPE,SIZE_TYPE>(const void* ptr)
//
// Which reads bytes from |ptr|, using |SIZE_TYPE| as the underlying
// sized-integer specifier (e.g. 'uint32_t'), and converting the result
// into a |HOST_TYPE| value. For example:
//
//     unpack<EGLImage,uint32_t>(ptr + 12);
//
// will read a 4-byte value from |ptr + 12| and convert it into
// an EGLImage, which is a host void*. The template detects host
// pointer types to perform proper type casting.
//
// TODO(digit): Add custom unpackers to handle generic opaque void* values.
//              and map them to unique 32-bit values.

template <typename T, typename S, bool IS_POINTER>
struct UnpackerT {};

template <typename T, typename S>
struct UnpackerT<T,S,false> {
    static inline T unpack(const void* ptr) {
        COMPILE_ASSERT(sizeof(T) == sizeof(S));
        return (T)(*(S*)(ptr));
    }
};

template <typename T, typename S>
struct UnpackerT<T,S,true> {
    static inline T unpack(const void* ptr) {
        return (T)(uintptr_t)(*(S*)(ptr));
    }
};

template <>
struct UnpackerT<float,uint32_t,false> {
    static inline float unpack(const void* ptr) {
        union {
            float f;
            uint32_t u;
        } v;
        v.u = *(uint32_t*)(ptr);
        return v.f;
    }
};

template <>
struct UnpackerT<double,uint64_t,false> {
    static inline double unpack(const void* ptr) {
        union {
            double d;
            uint32_t u;
        } v;
        v.u = *(uint64_t*)(ptr);
        return v.d;
    }
};

template <>
struct UnpackerT<ssize_t,uint32_t,false> {
    static inline ssize_t unpack(const void* ptr) {
        return (ssize_t)*(int32_t*)(ptr);
    }
};

template <typename T, typename S>
inline T Unpack(const void* ptr) {
    return UnpackerT<T, S, is_pointer<T>::value>::unpack(ptr);
}

// Helper class used to ensure input buffers passed to EGL/GL functions
// are properly aligned (preventing crashes with some backends).
// Usage example:
//
//    InputBuffer inputBuffer(ptr, size);
//    glDoStuff(inputBuffer.get());
//
// inputBuffer.get() will return the original value of |ptr| if it was
// aligned on an 8-byte boundary. Otherwise, it will return the address
// of an aligned heap-allocated copy of the original |size| bytes starting
// from |ptr|. The heap block is released at scope exit.
class InputBuffer {
public:
    InputBuffer(const void* input, size_t size, size_t align = 8) :
            mBuff(input), mIsCopy(false) {
        if (((uintptr_t)input & (align - 1U)) != 0) {
            void* newBuff = malloc(size);
            memcpy(newBuff, input, size);
            mBuff = newBuff;
            mIsCopy = true;
        }
    }

    ~InputBuffer() {
        if (mIsCopy) {
            free((void*)mBuff);
        }
    }

    const void* get() const {
        return mBuff;
    }

private:
    const void* mBuff;
    bool mIsCopy;
};

// Helper class used to ensure that output buffers passed to EGL/GL functions
// are aligned on 8-byte addresses.
// Usage example:
//
//    ptr = stream->alloc(size);
//    OutputBuffer outputBuffer(ptr, size);
//    glGetStuff(outputBuffer.get());
//    outputBuffer.flush();
//
// outputBuffer.get() returns the original value of |ptr| if it was already
// aligned on an 8=byte boundary. Otherwise, it returns the size of an heap
// allocated zeroed buffer of |size| bytes.
//
// outputBuffer.flush() copies the content of the heap allocated buffer back
// to |ptr| explictly, if needed. If a no-op if |ptr| was aligned.
class OutputBuffer {
public:
    OutputBuffer(unsigned char* ptr, size_t size, size_t align = 8) :
            mOrgBuff(ptr), mBuff(ptr), mSize(size) {
        if (((uintptr_t)ptr & (align - 1U)) != 0) {
            void* newBuff = calloc(1, size);
            mBuff = newBuff;
        }
    }

    ~OutputBuffer() {
        if (mBuff != mOrgBuff) {
            free(mBuff);
        }
    }

    void* get() const {
        return mBuff;
    }

    void flush() {
        if (mBuff != mOrgBuff) {
            memcpy(mOrgBuff, mBuff, mSize);
        }
    }
private:
    unsigned char* mOrgBuff;
    void* mBuff;
    size_t mSize;
};

}  // namespace emugl

#endif  // EMUGL_PROTOCOL_UTILS_H
