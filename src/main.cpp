#include "utils.hpp"
#include <GLFW/glfw3.h>
#include "shaders.hpp"
#include <stb_image.h>
#include <stb_image_write.h>

GLFWwindow* window;

static void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(1280, 720, "image store", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    glad_set_post_callback(glErrorCallback);

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    });

    glClearColor(0.1, 0.1, 0.1, 0);
    
    int w, h;
    u32 inTex = -1;
    {
        int nc;
        auto img = stbi_load("imgs/Windmill_NOAA.png", &w, &h, &nc, 3);
        
        if (img) {
            glGenTextures(1, &inTex);
            glBindTexture(GL_TEXTURE_2D, inTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            stbi_image_free(img);
        }
        else
            printf("Error loading img\n");
    }

    u32 outTex;
    {
        glGenTextures(1, &outTex);
        glBindTexture(GL_TEXTURE_2D, outTex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8UI, w, h);
    }

    u32 compProg = easyCreateComputeShaderProg("compute", shader_srcs::computeSrc);
    glUseProgram(compProg);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inTex);
    glBindImageTexture(0, outTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8UI);
    glUniform1i(0, 0);
    glUniform1i(1, 0);

    glDispatchCompute((w+15)/16, (h+15)/16, 1);

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT); // make sure the output image has been written
    glFinish();
    
    u8* img = new u8[w * h * 4];
    glBindTexture(GL_TEXTURE_2D, outTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, img);
    stbi_write_png("imgs/out.png", w, h, 1, img, w*4);
    delete[] img;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }
}