#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream> //file stream, 
#include <string>
#include <sstream>



struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

static struct ShaderProgramSource ParseShader(const std::string& filepath) {
    
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };
    ShaderType type = ShaderType::NONE;

    std::string line;
    std::stringstream ss[2];

    while (getline(stream, line)) 
    {
        if (line.find("#shader") != std::string::npos) 
        {
            if (line.find("vertex") != std::string::npos) 
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else 
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str() };
}

static void drawTriangle() {
    //Draw a triangle using legacy opengl
    //Place inside game loop
    //typically, you want to use modern opengl
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f( 0.0f,  0.5f);
    glVertex2f( 0.5f, -0.5f);
    glEnd();
}

static unsigned int compileShader(unsigned int type, const std::string source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //Error Handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result) ;
    if (result == GL_FALSE) { //error
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char)); //alloca() allocates memory within the current function's stack frame. Memory allocated using alloca() will be removed from the stack when the current function returns. alloca() is limited to small allocations.
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Failed to compile shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;

}
static unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
    
    unsigned int program_id = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program_id, vs);
    glAttachShader(program_id, fs);
    glLinkProgram(program_id);

    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program_id;
}

int main(void)
{
    GLFWwindow* window;

    //INIT GLFW
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "glewInit() Error" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl; //4.6.0 - Build 27.20.100.9621

    //VERTEX
    unsigned int buffer; 
    float positions[6] = { -0.5f, -0.5f, 0.0f,  0.5f, 0.5f, -0.5f };
    glGenBuffers(1, &buffer); //arg1: how many buffers would you like?
    glBindBuffer(GL_ARRAY_BUFFER, buffer); //arg1: defines the purpose, or how buffer will be used. The currently bound buffer is considered to be the "selected" buffer
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    //TELL OPENGL OUR LAYOUT
    glEnableVertexAttribArray(0); //arg1: the index that you want to enable. Enables the selected buffer
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, 0);
        //arg1: starting index
        //arg2: how many numbers are in 1 vertex
        //arg3: type of data
        //arg4: true = normalized (0 < x < 1), false = scalar (0 < x < 255)
        //arg5: stride: number of bytes for each vertex
        //arg6: wtf

    ShaderProgramSource source = ParseShader("Basic.shader");

    unsigned int shader = createShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);


    //GAME LOOP
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);


        glDrawArrays(GL_TRIANGLES, 0, 3); //use this function when you DON'T have an index buffer. arg1: type. arg2: starting index. arg3: vertex count (2 coordinate = 1 vertex);


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}


/*

libaraies
    All libraries has 2 essential parts
    1. An 'include' folder the contains all the .h files
    2. A 'lib' folder that contains .lib files.

glew
    openGL Extension Wrangler.
     - We need a way to call the opengl functions.
     - Rembember their implementations are define on your graphics card
     - glew returns function pointers from your GPU
    Download: http://glew.sourceforge.net/index.html
     - I used the binary version as opposed to the source code
     - I stored in here: C:\Users\devon\VisualStudio\solution_game\Dependencies\glew-2.1.0
    Documentation: http://glew.sourceforge.net/basic.html
    VS Properties Page
     - C/C++  -> General -> Additional Include Directores  -> $(SolutionDir)Dependencies\glew-2.1.0\include\GL
     - Linker -> General -> Additional Library Directories -> $(SolutionDir)Dependencies\glew-2.1.0\lib\Release\x64
     - Linker -> Input   -> Additional Dependencies -> glew32s.lib
     - C/C++  -> Preprocessor -> Preprocessor Definitions -> GLEW_STATIC
    Gochas
     - call glewInit() AFTER glfwMakeContextCurrent(window);
     - #include <GL/glew.h> BEFORE #include <GLFW/glfw3.h>

Vertex
    Array of bytes in memory, just of buffer
    stored in the GPU
Shader
    Program that runs on the GPU and draws images
    Vertex Shader - gets called for each vertex, position, called first
    Fragment Shader - aka Pixel Shaders, called after vertex shader. Runs for each pixel which makes operations expensive. Decides which COLOR each pixel should be.

To render an image, you select 1 Vertex & 1 shader

*/


/*
VISUAL STUDIO HOT KEYS
 - ctr + D  Duplicate a line
 - ctr + C  Copy entire line (when nothing is selected)
 - ctr + X  Cut entire line
 - ctr + H  Find and replace
 - alt + up/down            move line of code
 - alt + shift + up/down    multiple cursors
*/