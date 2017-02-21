//
// Created by Miles Gibson on 20/02/17.
//

#include <string>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include "load_shader.h"



namespace glhelpers {

    GLuint LoadShaders(const char* logName, const char* vertexShaderCode, const char* fragmentShaderCode){

        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
//        std::string VertexShaderCode(vertexShaderCode);
//        std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
//        if(VertexShaderStream.is_open()){
//            std::string Line = "";
//            while(getline(VertexShaderStream, Line))
//                VertexShaderCode += "\n" + Line;
//            VertexShaderStream.close();
//        }else{
//            printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
//            getchar();
//            return 0;
//        }

        // Read the Fragment Shader code from the file
//        std::string FragmentShaderCode(fragmentShaderCode);
//        std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
//        if(FragmentShaderStream.is_open()){
//            std::string Line = "";
//            while(getline(FragmentShaderStream, Line))
//                FragmentShaderCode += "\n" + Line;
//            FragmentShaderStream.close();
//        }

        GLint Result = GL_FALSE;
        int InfoLogLength;


        // Compile Vertex Shader
        printf("Compiling vertex shader : %s\n", logName);
        char const * VertexSourcePointer = vertexShaderCode;
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
        glCompileShader(VertexShaderID);

        // Check Vertex Shader
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            fprintf(stderr, "%s\n", &VertexShaderErrorMessage[0]);
        }



        // Compile Fragment Shader
        printf("Compiling fragment shader : %s\n", logName);
        char const * FragmentSourcePointer = fragmentShaderCode;
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
        glCompileShader(FragmentShaderID);

        // Check Fragment Shader
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            fprintf(stderr, "%s\n", &FragmentShaderErrorMessage[0]);
            fprintf(stderr, "CODE: %s\n", fragmentShaderCode);
        }



        // Link the program
        printf("Linking program\n");
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }


        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
    }
}
