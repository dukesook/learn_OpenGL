#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream> //file stream, 
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
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

static void GLClearError() {
    while (glGetError() != GL_NO_ERROR) {
        //keep getting errors
    }
}
static bool GLLogCall(const char* function, const char* file, unsigned int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << ")" << std::endl;
        std::cout << "\tfunction: " << function << std::endl;
        std::cout << "\tfile:     " << file << std::endl;
        std::cout << "\tline:     " << line << std::endl;
        return false;
    }
    return true;
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

    glfwSwapInterval(1); //synchornizes with monitor refresh rate

    if (glewInit() != GLEW_OK) {
        std::cout << "glewInit() Error" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl; //4.6.0 - Build 27.20.100.9621

    //VERTEX  
    float positions[] = { 
        -0.5f, -0.5f, //0
         0.5f, -0.5f, //1
         0.5f,  0.5f, //2
        -0.5f,  0.5f   //3
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    unsigned int buffer;
    unsigned bufferCount = 1;
    unsigned int positionsVertexCount = 4;
    unsigned numbersPerVertex = 2;
    unsigned int positionsSize = positionsVertexCount * numbersPerVertex * sizeof(float);
    GLCall(glGenBuffers(bufferCount, &buffer)); //arg1: Number of buffers to generate. arg2: Location where buffers will be stored
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); //arg1: defines the purpose, or how buffer will be used. The currently bound buffer is considered to be the "selected" buffer
    GLCall(glBufferData(GL_ARRAY_BUFFER, positionsSize, positions, GL_STATIC_DRAW)); //links a buffer to its data

    //TELL OPENGL OUR LAYOUT
    int startingIndex = 0;
    bool normalized = false;
    int stride = sizeof(float) * numbersPerVertex;
    GLCall(glEnableVertexAttribArray(0)); //Enables the selected buffer. arg1: the index that you want to enable.
    GLCall(glVertexAttribPointer(startingIndex, numbersPerVertex, GL_FLOAT, normalized, stride, 0)); //defines an array of generic vertex attribute data //arg1: starting index. arg2: how many numbers are in 1 vertex. arg3: type of data. arg4: true = normalized (0 < x < 1), false = scalar (0 < x < 255). arg5: stride: number of bytes for each vertex. arg6: wtf

    unsigned int ibo;
    unsigned int indicesVertexCount = 6;
    unsigned int indicesSize = indicesVertexCount * sizeof(unsigned int);
    GLCall(glGenBuffers(bufferCount, &ibo)); //arg1: how many buffers would you like?
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)); //arg1: defines the purpose, or how buffer will be used. The currently bound buffer is considered to be the "selected" buffer
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW)); //links a buffer to its data

    //SHADERS
    ShaderProgramSource source = ParseShader("Basic.shader");
    unsigned int shader = createShader(source.VertexSource, source.FragmentSource); //Vertex Shaders: ???? Fragment Shaders: Tell opengl which color to use
    GLCall(glUseProgram(shader)); //"binding the shader"

    //UNIFORM
    GLCall(int location = glGetUniformLocation(shader, "u_Color")); //getting the location of the "u_Color" variable located in the shader
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f)); //Set the data in the shader. 4f = 4 floats in a vertex

    float red = 0.0f;
    float increment = 0.05f;

    //GAME LOOP
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glUniform4f(location, red, 0.3f, 0.8f, 1.0f)); //Set the data in the shader. 4f = 4 floats in a vertex
        //glDrawArrays(GL_TRIANGLES, 0, 6); //use this function when you DON'T have an index buffer. arg1: type. arg2: starting index. arg3: vertex count (2 coordinate = 1 vertex);
        GLCall(glDrawElements(GL_TRIANGLES, indicesVertexCount, GL_UNSIGNED_INT, nullptr));

        if (red > 1.0f) {
            increment = -0.05f;
        } else if (red < 0.0f) {
            increment = 0.05f;
        }

        red += increment;

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }

    GLCall(glDeleteProgram(shader));

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
Uniforms & Attributes - The Cherno #11
    Both send data to the GPU
    Uniforms
        Set per draw
    Attributes
        Set per vertex


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