// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include "load_shader.h"

namespace {

    struct GlWindowConfig {
        int width, height;
        const char* title;
    };

    template <typename Setup, typename Render>
    int with_gl_window(const GlWindowConfig& cfg, Setup&& setup, Render&& render) {
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

        GLFWwindow* window;

        /* Initialize the library */
        if (!glfwInit())
            return -1;

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

        setup(window);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            render(window);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        glfwTerminate();
        return 0;
    }


}

int main(void)
{

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
                    640, 480, "elfw-gl"
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
                programID = glhelpers::LoadShaders( "shaders/basic.vertexshader", "shaders/basic.fragmentshader" );

            },
            [&](GLFWwindow* window) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
            }
    );
}
