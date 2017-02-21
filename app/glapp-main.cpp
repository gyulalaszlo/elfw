// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <cstdio>

#include <GL/glew.h>
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif
#include <GLFW/glfw3.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

#include "load_shader.h"
#include "../tools/elfw-resources.h"
#include "perf.h"


namespace {

    // error callback
    void errorcb(int error, const char* desc)
    {
        fprintf(stderr, "[GLFW] error %d: %s\n", error, desc);
    }

    struct GlWindowConfig {
        int width, height;
        const char* title;
        bool vSync;
        int samples;
    };

    struct NVGA {

        NVGA(const GlWindowConfig& cfg) {
            initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
            initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
            initGraph(&gpuGraph, GRAPH_RENDER_MS, "GPU Time");


            if (cfg.samples > 1) {
                vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
            } else {
                vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
            }

            if (vg == NULL) {
                printf("Could not init nanovg.\n");
                exit(-21);
            }

            initGPUTimer(&gpuTimer);

            glfwSetTime(0);
            prevt = glfwGetTime();
        }

        ~NVGA() {

            nvgDeleteGL3(vg);

            printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
            printf("          CPU Time: %.2f ms\n", getGraphAverage(&cpuGraph) * 1000.0f);
            printf("          GPU Time: %.2f ms\n", getGraphAverage(&gpuGraph) * 1000.0f);
        }


        template <typename F>
        void draw(GLFWwindow* window, F&& innerRender ) {

            double mx, my, t, dt;
            int winWidth, winHeight;
            int fbWidth, fbHeight;
            float pxRatio;
            float gpuTimes[3];
            int i, n;

            t = glfwGetTime();
            dt = t - prevt;
            prevt = t;

            startGPUTimer(&gpuTimer);

            glfwGetCursorPos(window, &mx, &my);
            glfwGetWindowSize(window, &winWidth, &winHeight);
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            // Calculate pixel ration for hi-dpi devices.
            pxRatio = (float)fbWidth / (float)winWidth;

            // Update and render
            glViewport(0, 0, fbWidth, fbHeight);

            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);


            nvgBeginFrame(vg, winWidth, winHeight, pxRatio);


            innerRender(window, vg);
            renderGraph(vg, 5,5, &fps);
            renderGraph(vg, 5+200+5,5, &cpuGraph);
            if (gpuTimer.supported)
                renderGraph(vg, 5+200+5+200+5,5, &gpuGraph);

            nvgEndFrame(vg);

            // Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
            cpuTime = glfwGetTime() - t;

            updateGraph(&fps, dt);
            updateGraph(&cpuGraph, cpuTime);

            // We may get multiple results.
            n = stopGPUTimer(&gpuTimer, gpuTimes, 3);
            for (i = 0; i < n; i++)
                updateGraph(&gpuGraph, gpuTimes[i]);

//            if (screenshot) {
//                screenshot = 0;
//                saveScreenShot(fbWidth, fbHeight, premult, "dump.png");
//            }
        }

        NVGcontext* vg = NULL;
        GPUtimer gpuTimer;
        PerfGraph fps, cpuGraph, gpuGraph;
        double prevt = 0, cpuTime = 0;
    };

    template <typename Setup, typename Render>
    int with_gl_window(const GlWindowConfig& cfg, Setup&& setup, Render&& render) {
        GLFWwindow* window;

        /* Initialize the library */
        if (!glfwInit())
            return -1;

        glfwSetErrorCallback(errorcb);

        if (cfg.samples > 1) {
            glfwWindowHint(GLFW_SAMPLES, cfg.samples);
        }

#ifndef _WIN32 // don't require this on win32, and works with more cards
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(cfg.width, cfg.height, cfg.title, NULL, NULL);
        if (!window)
        {
            fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
            glfwTerminate();
            return -1;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(window);


        glewExperimental=GL_TRUE; // Needed in core profile

        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            return -1;
        }

        // GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
        glGetError();

        // VSync
        if (cfg.vSync) glfwSwapInterval(1);

        {
            NVGA nvga(cfg);

            setup(window);

            /* Loop until the user closes the window */
            while (!glfwWindowShouldClose(window))
            {
                nvga.draw(window, [&](GLFWwindow* w, NVGcontext* nvg){
                    /* Render here */
                    render(w, nvg);
                });

                /* Swap front and back buffers */
                glfwSwapBuffers(window);

                /* Poll for and process events */
                glfwPollEvents();
            }

        }

        glfwTerminate();
        return 0;
    }



}

int main(void)
{
    elfw::ResourceLoader loader("shaders.data");

    auto vs = loader.get("shaders/basic.vertexshader");
    auto fs = loader.get("shaders/basic.fragmentshader");


    GLuint VertexArrayID;
    // This will identify our vertex buffer
    GLuint vertexbuffer;
    GLuint programID;
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            0.0f,  1.0f, 0.0f,
    };

    with_gl_window(
            {
                    640, 480, "elfw-gl", true, 4
            },
            [&](GLFWwindow* window) {

                glGenVertexArrays(1, &VertexArrayID);
                glBindVertexArray(VertexArrayID);


                // Generate 1 buffer, put the resulting identifier in vertexbuffer
                glGenBuffers(1, &vertexbuffer);
                // The following commands will talk about our 'vertexbuffer' buffer
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                // Give our vertices to OpenGL.
                glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


                // load the shaders
                // Create and compile our GLSL program from the shaders
                programID = glhelpers::LoadShaders( "basic",  vs.ptr, fs.ptr );

            },
            [&](GLFWwindow* window, NVGcontext* vg) {
//                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                nvgBeginPath(vg);
                nvgCircle(vg, 50, 150, 40 );
                nvgFillColor(vg, nvgRGB(0xff, 0, 0x77));
                nvgFill(vg);
                /*
                // Use our shader
                glUseProgram(programID);
                // 1rst attribute buffer : vertices
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                glVertexAttribPointer(
                        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                );

                // Draw the triangle !
                glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
                glDisableVertexAttribArray(0);
                 */
            }
    );
}
