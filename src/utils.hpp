#pragma once

#include <stdio.h>
#include <assert.h>
#include <string.h>
#define GLAD_DEBUG
#include <glad/glad.h>
#include <span>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef const char* const ConstStr;

constexpr int SCRATCH_BUFFER_SIZE = 4 * 1024 * 1024;
extern char buffer[SCRATCH_BUFFER_SIZE];
extern std::span<u8> bufferU8;
template <typename T> auto bufferSpan(size_t offset = 0) { return std::span<T>((T*)(buffer + offset), (SCRATCH_BUFFER_SIZE - offset) / sizeof(T)); }
typedef const char* const ConstStr;

void glErrorCallback(const char* name, void* funcptr, int len_args, ...);

char* checkCompileErrors(u32 shad, std::span<char> buffer);
char* checkLinkErrors(u32 prog, std::span<char> buffer);
void printShaderCodeWithHeader(const char* src);
u32 easyCreateShader(const char* name, const char* src, GLenum type);
u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc);
u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc, u32 vertShad, u32 fragShad);
u32 easyCreateComputeShaderProg(const char* name, const char* src);

// -- DEFER --
template <typename F>
struct _Defer {
    F f;
    _Defer(F f) : f(f) {}
    ~_Defer() { f(); }
};

template <typename F>
_Defer<F> _defer_func(F f) {
    return _Defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = _defer_func([&](){code;})