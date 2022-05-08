#include "utils.hpp"
#include <span>
#include "shaders.hpp"

char buffer[SCRATCH_BUFFER_SIZE];
std::span<u8> bufferU8((u8*)buffer, SCRATCH_BUFFER_SIZE);

static const char* geGlErrStr(GLenum const err)
{
	switch (err) {
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
	default:
		assert(!"unknown error");
		return nullptr;
	}
}

void glErrorCallback(const char* name, void* funcptr, int len_args, ...) {
	GLenum error_code;
	error_code = glad_glGetError();
	if (error_code != GL_NO_ERROR) {
		fprintf(stderr, "ERROR %s in %s\n", geGlErrStr(error_code), name);
		assert(false);
	}
}

// --- shader utils ---

char* checkCompileErrors(u32 shad, std::span<char> buffer)
{
    i32 ok;
    glGetShaderiv(shad, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLsizei outSize;
        glGetShaderInfoLog(shad, buffer.size(), &outSize, buffer.data());
        return buffer.data();
    }
    return nullptr;
}

char* checkLinkErrors(u32 prog, std::span<char> buffer)
{
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, buffer.size(), nullptr, buffer.data());
        return buffer.data();
    }
    return nullptr;
}

static void printCodeWithLines(std::span<const char*> srcs)
{
    for (const char* s : srcs)
    {
        int line = 1;
        int start = 0;
        int end = 0;
        auto printLine = [&]() { printf("%4d| %.*s\n", line, end - start, s + start); };
        while (s[end]) {
            if (s[end] == '\n') {
                printLine();
                start = end = end + 1;
                line++;
            }
            else
                end++;
        }
        printLine();
    }
}

void printShaderCodeWithHeader(const char* src)
{
    const char* srcs[2] = { shader_srcs::header, src };
    printCodeWithLines(srcs);
}

u32 easyCreateShader(const char* name, const char* src, GLenum type)
{
    static ConstStr s_shaderTypeNames[] = { "VERT", "FRAG", "GEOM", "COMP"};
    const char* typeName = nullptr;
    switch (type) {
    case GL_VERTEX_SHADER:
        typeName = "VERT"; break;
    case GL_FRAGMENT_SHADER:
        typeName = "FRAG"; break;
    case GL_GEOMETRY_SHADER:
        typeName = "GEOM"; break;
    case GL_COMPUTE_SHADER:
        typeName = "COMP"; break;
    default:
        assert(false);
    }

    const u32 shad = glCreateShader(type);
    ConstStr srcs[] = { shader_srcs::header, src };
    glShaderSource(shad, 2, srcs, nullptr);
    glCompileShader(shad);
    if (const char* errMsg = checkCompileErrors(shad, buffer)) {
        printf("Error in '%s'(%s):\n%s", name, typeName, errMsg);
        printShaderCodeWithHeader(src);
        assert(false);
    }
    return shad;
}

u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc, u32 vertShad, u32 fragShad)
{
    u32 prog = glCreateProgram();

    glAttachShader(prog, vertShad);
    glAttachShader(prog, fragShad);
    defer(
        glDetachShader(prog, vertShad);
        glDetachShader(prog, fragShad);
    );

    glLinkProgram(prog);
    if (const char* errMsg = checkLinkErrors(prog, buffer)) {
        printf("%s\n", errMsg);
        printf("Vertex Shader:\n");
        printShaderCodeWithHeader(vertShadSrc);
        printf("Fragment Shader:\n");
        printShaderCodeWithHeader(fragShadSrc);
        assert(false);
    }

    return prog;
}

u32 easyCreateShaderProg(const char* name, const char* vertShadSrc, const char* fragShadSrc)
{
    const u32 vertShad = easyCreateShader(name, vertShadSrc, GL_VERTEX_SHADER);
    const u32 fragShad = easyCreateShader(name, fragShadSrc, GL_FRAGMENT_SHADER);
    const u32 prog = easyCreateShaderProg(name, vertShadSrc, fragShadSrc, vertShad, fragShad);
    glDeleteShader(vertShad);
    glDeleteShader(fragShad);
    return prog;
}

u32 easyCreateComputeShaderProg(const char* name, const char* src)
{
    u32 shad = easyCreateShader("compute", src, GL_COMPUTE_SHADER);
    u32 prog = glCreateProgram();
    glAttachShader(prog, shad);
    glLinkProgram(prog);
    if (const char* errMsg = checkLinkErrors(prog, buffer)) {
        printf("%s\n", errMsg);
        printf("Compute Shader:\n");
        printShaderCodeWithHeader(src);
        assert(false);
    }
    glDetachShader(prog, shad);
    glDeleteShader(shad);
    return prog;
}