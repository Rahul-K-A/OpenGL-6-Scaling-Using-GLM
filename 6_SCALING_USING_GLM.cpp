#include <iostream>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include<glm\gtc\matrix_transform.hpp>

//refer http://learnwebgl.brown37.net/transformations2/transformations_matrices.html#transformation-matrices

GLint Width = 640, Height = 480;

GLuint Vao, Vbo, Shader;

GLuint  UniformModel;
//Offsets and edge detecting booleans
bool IsAtEdgeX = false;
bool IsAtEdgeY = false;
float OffsetX = 0.001f;
float OffsetY = 0.001f;
//Increment to offset which happens every loop
float IncrementX = 0.0001f;
float IncrementY = 0.0001f;
//Variable to facilitate conversion to radians
const float ToRadians = 22.f / (7.f * 180.f);
//Variable to change angle within main
float CurrentAngle = 0.f;
float AngleIncrement = 0.1f;

//Maximum offset for translation
float MaxOffsetX = 1.f;
float MaxOffsetY = 1.f;

//Vertex Shader
//Since we arent scaling within GLSL anymore we can create a vec4 out of vec3 pos and one extra point

static const char* vShader = R"gl(
  #version 330
  layout (location = 0) in vec3 position;
  uniform mat4 model;
  void main()
  {
    gl_Position = model* vec4(position,1);
  }
)gl";

//Fragment shader
static const char* fShader = R"gl(                                             
  #version 330
  out vec4 colour;
  void main()
  {
    colour=vec4(1.f,0.f,0.f,1.f);
  }
)gl";


void CreateTriangle()
{
    float Vertices[] = {
        -1.f,-1.f,0.f,
         1.f,-1.f,0.f,
         0.f,1.f,0.f,
    };

    glGenVertexArrays(1, &Vao);
    glBindVertexArray(Vao);
    glGenBuffers(1, &Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



GLuint AddShader(GLuint TheProgram, const char* ShaderCode, GLenum ShaderType)
{
    GLuint TheShader = glCreateShader(ShaderType);
    const GLchar* TheCode[1];
    TheCode[0] = ShaderCode;
    GLint CodeLength[1];
    CodeLength[0] = strlen(ShaderCode);
    glShaderSource(TheShader, 1, TheCode, CodeLength);
    glCompileShader(TheShader);
    GLint result = 0;
    GLchar eLog[1024] = { 0 };
    glGetShaderiv(TheShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetShaderInfoLog(TheShader, sizeof(eLog), NULL, eLog);
        std::cout << "Error compiling shader: " << eLog << std::endl;
        return NULL;
    }
    glAttachShader(TheProgram, TheShader);
    return TheShader;
}


void CompileShaders()
{
    Shader = glCreateProgram();
    if (!Shader) {
        std::cout << "Program was not sucessfully created!!!\n";
        exit(1);
    }
    GLuint vShaderLocation = AddShader(Shader, vShader, GL_VERTEX_SHADER);
    GLuint fShaderLocation = AddShader(Shader, fShader, GL_FRAGMENT_SHADER);
    GLint result = 0;
    GLchar eLog[1024] = { 0 };
    glLinkProgram(Shader);
    glDeleteShader(vShaderLocation);
    glDeleteShader(fShaderLocation);
    glGetProgramiv(Shader, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetProgramInfoLog(Shader, sizeof(eLog), NULL, eLog);
        std::cout << "Error linking program: " << eLog << std::endl;
        return;
    }
    glValidateProgram(Shader);
    glGetProgramiv(Shader, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(Shader, sizeof(eLog), NULL, eLog);
        std::cout << "Error validating program " << eLog << std::endl;
        return;
    }
    //glGetUniformLocation returns an integer that represents the location of a specific uniform variable within a program object. 
    //Should only do this after compiling and linking shader to program
    UniformModel = glGetUniformLocation(Shader, "model");

}

int main()
{
    if (glfwInit() != GL_TRUE)
    {
        std::cout << "GLFW INITIALIZATION FAILED\n";
        glfwTerminate();
        return 1;
    }

    //GLFW Window Properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* MainWindow = glfwCreateWindow(Width, Height, "TEST WINDOW", NULL, NULL);
    if (!MainWindow)
    {
        printf("Window Creation Failed!!!!\n");
        glfwTerminate();
        return 2;
    }
    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(MainWindow, &BufferWidth, &BufferHeight);
    glfwMakeContextCurrent(MainWindow);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        printf("GLEW INITIALIZATION FAILED!!!\n");
        glfwTerminate();
        return 1;
    }
    glViewport(0, 0, BufferWidth, BufferHeight);
    CreateTriangle();
    CompileShaders();
    while (!glfwWindowShouldClose(MainWindow))
    {
        
        glfwPollEvents();
        //Setting offsets
        OffsetX += IncrementX;
        OffsetY += IncrementY;
        if (abs(OffsetX) > MaxOffsetX)
        {
            IsAtEdgeX = !IsAtEdgeX;
            IncrementX *= -1;
        }
        if (abs(OffsetY) > MaxOffsetY)
        {
            IsAtEdgeY = !IsAtEdgeY;
            IncrementY *= -1;
        }
        CurrentAngle += AngleIncrement;
        if (CurrentAngle > 360.f)
            CurrentAngle = 0.f;
        //Creating identity matrix to store the transforms
        glm::mat4 model(1.f);
        //model = glm::rotate(model, CurrentAngle * ToRadians, glm::vec3(0.f, 0.f, 1.f));
       // model = glm::translate(model, glm::vec3(OffsetX, OffsetY, 0.f));
        model = glm::scale(model, glm::vec3(OffsetX,1.f, 1.f));
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(Shader);
        //Passing the value into shader
        glUniformMatrix4fv(UniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(Vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);
        glfwSwapBuffers(MainWindow);

    }
    glDeleteProgram(Shader);
    glDeleteVertexArrays(1, &Vao);
    glDeleteBuffers(1, &Vbo);
    glfwTerminate();

}
