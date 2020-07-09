//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

static const GLsizei WIDTH = 1080, HEIGHT = 720; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;
static int g_shaderProgram = 0;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(0.0f, 0.0f, 5.0f));

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
  case GLFW_KEY_1:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    break;
  case GLFW_KEY_2:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    break;
	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    g_captureMouse = !g_captureMouse;


  if (g_captureMouse)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g_capturedMouseJustNow = true;
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = float(xpos);
    lastY = float(ypos);
    firstMouse = false;
  }

  GLfloat xoffset = float(xpos) - lastX;
  GLfloat yoffset = lastY - float(ypos);  

  lastX = float(xpos);
  lastY = float(ypos);

  if (g_captureMouse)
    camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
  if (keys[GLFW_KEY_W])
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (keys[GLFW_KEY_A])
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (keys[GLFW_KEY_S])
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (keys[GLFW_KEY_D])
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

GLsizei CreateCylinder(GLuint& vao, float radius, int numberSlices, float height)
{
    int numberTriangles = numberSlices * 2 + numberSlices * 2;
    int numberVertices = numberSlices * 2;
    int numberIndices = numberTriangles * 3;

    float angleStep = (360.0f / numberSlices) * 3.14159265358979323846f / 180;
    float4 center = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 top = float4(center.x, center.y + height, center.z, center.w);

    std::vector<float> positions = { center.x, center.y, center.z, center.w};
    std::vector<float> normals = { 0.0f, -1.0f, 0.0f, 1.0f,
                                    0.0f, 1.0f, 0.0f, 1.0f };
    std::vector<uint32_t> indices;

    float4 angleVector = float4(0.0f, 0.0f, radius, 1.0f);
    float4 edge;
    float newX;
    float newZ;

    for (int i = 0; i < numberSlices; i++)
    {
        edge = center + angleVector;
        edge.w = 1.0f;
        positions.push_back(edge.x);
        positions.push_back(edge.y);
        positions.push_back(edge.z);
        positions.push_back(edge.w);
        positions.push_back(edge.x);
        positions.push_back(edge.y + height);
        positions.push_back(edge.z);
        positions.push_back(edge.w);

        newZ = angleVector.z * cosf(angleStep) + (-1 * sinf(angleStep) * angleVector.x);
        newX = angleVector.z * sinf(angleStep) + cosf(angleStep) * angleVector.x;
        angleVector.x = newX;
        angleVector.z = newZ;
    }
    positions.push_back(top.x);
    positions.push_back(top.y);
    positions.push_back(top.z);
    positions.push_back(top.w);


    for (uint32_t i = 0; i < numberVertices - 3; i = i + 2)
    {
        indices.push_back(0u);
        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
        indices.push_back(i + 4);
        indices.push_back(i + 2);
        indices.push_back(i + 4);
        indices.push_back(numberVertices + 1);
    }
    indices.push_back(0u);
    indices.push_back(numberVertices - 1);
    indices.push_back(1u);

    indices.push_back(numberVertices - 1);
    indices.push_back(1u);
    indices.push_back(numberVertices);

    indices.push_back(1u);
    indices.push_back(numberVertices);
    indices.push_back(2u);

    indices.push_back(numberVertices);
    indices.push_back(2u);
    indices.push_back(numberVertices + 1);

    float4 side0;
    float4 side1 = top - center;
    float4 normal;
    for (int i = 4; i <= positions.size() - 20; i = i + 8)
    {
        side0 = float4(positions.at(i + 8), positions.at(i + 9), positions.at(i + 10), positions.at(i + 11)) - float4(positions.at(i), positions.at(i + 1), positions.at(i + 2), positions.at(i + 3));
        normal = scal(side0, side1);
        normals.push_back(normal.x);
        normals.push_back(normal.y);
        normals.push_back(normal.z);
        normals.push_back(normal.w);
    }
    side0 = float4(positions.at(4), positions.at(5), positions.at(6), positions.at(7)) - float4(positions.at(numberSlices * 4), positions.at(numberSlices * 4 + 1), positions.at(numberSlices * 4 + 2), positions.at(numberSlices * 4 + 3));
    normal = scal(side0, side1);
    normals.push_back(normal.x);
    normals.push_back(normal.y);
    normals.push_back(normal.z);
    normals.push_back(normal.w);

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCone(GLuint& vao, float radius, int numberSlices, float height)
{
    int numberTriangles = numberSlices * 2;
    int numberVertices = numberSlices + 1;
    int numberIndices = numberTriangles * 3;

    float angleStep = (360.0f / numberSlices) * 3.14159265358979323846f / 180;
    float4 center = float4(0.0f, 0.0f, 0.0f, 1.0f);

    std::vector<float> positions = { center.x, center.y, center.z, center.w };
    std::vector<float> normals = { 0.0f, -1.0f, 0.0f, 1.0f };
    std::vector<uint32_t> indices;

    float4 angleVector = float4(0.0f, 0.0f, radius, 1.0f);
    float4 edge;
    float newX;
    float newZ;
    for (int i = 0; i < numberSlices; i++)
    {
        edge = center + angleVector;
        edge.w = 1.0f;
        positions.push_back(edge.x);
        positions.push_back(edge.y);
        positions.push_back(edge.z);
        positions.push_back(edge.w);

        newZ = angleVector.z * cosf(angleStep) + (-1 * sinf(angleStep) * angleVector.x);
        newX = angleVector.z * sinf(angleStep) + cosf(angleStep) * angleVector.x;
        angleVector.x = newX;
        angleVector.z = newZ;
    }
    float4 top = float4(center.x, center.y + height, center.z, center.w);
    positions.push_back(top.x);
    positions.push_back(top.y);
    positions.push_back(top.z);
    positions.push_back(top.w);


    for (uint32_t i = 0; i < numberSlices - 1; i++)
    {
        indices.push_back(0u);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }
    indices.push_back(0u);
    indices.push_back((uint32_t)numberSlices);
    indices.push_back(1u);
    for (uint32_t i = 0; i < numberSlices - 1; i++)
    {
        indices.push_back(i + 1);
        indices.push_back(i + 2);
        indices.push_back((uint32_t)numberVertices);
    }
    indices.push_back((uint32_t)numberSlices);
    indices.push_back(1u);
    indices.push_back((uint32_t)numberVertices);

    float4 side0;
    float4 side1;
    float4 normal;
    uint32_t i = 4;
    while (i <= positions.size() - 12)
    {
        side0 = float4(positions.at(i), positions.at(i + 1u), positions.at(i + 2u), positions.at(i + 3u)) - top;
        side1 = float4(positions.at(i + 4u), positions.at(i + 5u), positions.at(i + 6u), positions.at(i + 7u)) - top;
        normal = scal(side0, side1);
        normals.push_back(normal.x);
        normals.push_back(normal.y);
        normals.push_back(normal.z);
        normals.push_back(normal.w);
        i += 4;
    }
    side0 = float4(positions.at(4u), positions.at(5u), positions.at(6u), positions.at(7u)) - top;

    normal = scal(side1, side0);
    normals.push_back(normal.x);
    normals.push_back(normal.y);
    normals.push_back(normal.z);
    normals.push_back(normal.w);

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateSphere(float radius, int numberSlices, GLuint &vao)
{
  int i, j;

  int numberParallels = numberSlices;
  int numberVertices = (numberParallels + 1) * (numberSlices + 1);
  int numberIndices = numberParallels * numberSlices * 3;

  float angleStep = (2.0f * 3.14159265358979323846f) / ((float) numberSlices);
  //float helpVector[3] = {0.0f, 1.0f, 0.0f};

  std::vector<float> pos(numberVertices * 4, 0.0f);
  std::vector<float> norm(numberVertices * 4, 0.0f);
  std::vector<float> texcoords(numberVertices * 2, 0.0f);

  std::vector<int> indices(numberIndices, -1);

  for (i = 0; i < numberParallels + 1; i++)
  {
    for (j = 0; j < numberSlices + 1; j++)
    {
      int vertexIndex = (i * (numberSlices + 1) + j) * 4;
      int normalIndex = (i * (numberSlices + 1) + j) * 4;
      int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

      pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float) i) * sinf(angleStep * (float) j);
      pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float) i);
      pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float) i) * cosf(angleStep * (float) j);
      pos.at(vertexIndex + 3) = 1.0f;

      norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
      norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
      norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
      norm.at(normalIndex + 3) = 1.0f;

      texcoords.at(texCoordsIndex + 0) = (float) j / (float) numberSlices;
      texcoords.at(texCoordsIndex + 1) = (1.0f - (float) i) / (float) (numberParallels - 1);
    }
  }

  int *indexBuf = &indices[0];

  for (i = 0; i < numberParallels; i++)
  {
    for (j = 0; j < numberSlices; j++)
    {
      *indexBuf++ = i * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

      *indexBuf++ = i * (numberSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
      *indexBuf++ = i * (numberSlices + 1) + (j + 1);

      int diff = int(indexBuf - &indices[0]);
      if (diff >= numberIndices)
        break;
    }
    int diff = int(indexBuf - &indices[0]);
    if (diff >= numberIndices)
      break;
  }

  GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vboIndices);

  glBindVertexArray(vao);

  glGenBuffers(1, &vboVertices);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
  glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &vboNormals);
  glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
  glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(1);

  glGenBuffers(1, &vboTexCoords);
  glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
  glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);

  return indices.size();
}

GLsizei CreatePlane(GLuint& vao, float4 a, float size)
{
    std::vector<float> positions = {
        a.x, a.y, a.z, a.w,
        a.x + size, a.y, a.z, a.w,
        a.x, a.y + size, a.z, a.w,
        a.x + size, a.y + size, a.z, a.w
    };

    std::vector<float> normals = { 0.0f, 0.0f, 1.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f, 1.0f,
                                    0.0f, 0.0f, 1.0f, 1.0f };

    std::vector<uint32_t> indices = { 0u, 1u, 2u,
                                      1u, 2u, 3u};

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCuboid(GLuint& vao, float4 a, float ax, float ay, float az)
{
    std::vector<float> positions = {
        a.x, a.y, a.z, a.w,
        a.x + ax, a.y, a.z, a.w,
        a.x, a.y, a.z + az, a.w,
        a.x + ax, a.y, a.z + az, a.w,
        a.x, a.y + ay, a.z, a.w,
        a.x + ax, a.y + ay, a.z, a.w,
        a.x, a.y + ay, a.z + az, a.w,
        a.z + ax, a.y + ay, a.z + az, a.w
    };
    std::vector<uint32_t> indices = {
        0u, 1u, 3u,
        0u, 3u, 2u,
        0u, 4u, 5u,
        0u, 5u, 1u,
        1u, 5u, 3u,
        5u, 7u, 3u,
        0u, 4u, 6u,
        0u, 6u, 2u,
        2u, 6u, 3u,
        6u, 7u, 3u,
        4u, 5u, 6u,
        5u, 7u, 6u
    };

    std::vector<float> normals = {
        0.0f, -1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateTriangle(GLuint& vao)
{
  std::vector<float> positions = { -1.0f, 0.0f, 0.0f, 1.0f,
                                    1.0f, 0.0f, 0.0f, 1.0f,
                                    0.0f, 2.0f, 0.0f, 1.0f };

  std::vector<float> normals = {  0.0f, 0.0f, 1.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f, 1.0f };

  std::vector<float> texCoords = { 0.0f, 0.0f,
                                   0.5f, 1.0f,
                                   1.0f, 0.0f };

  std::vector<uint32_t> indices = { 0u, 1u, 2u };

  GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vboIndices);

  glBindVertexArray(vao);

  glGenBuffers(1, &vboVertices);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &vboNormals);
  glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(1);

  glGenBuffers(1, &vboTexCoords);
  glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
  glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);

  return indices.size();
}


int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Controls: "<< std::endl;
  std::cout << "press right mouse button to capture/release mouse cursor  "<< std::endl;
  std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
  std::cout << "press ESC to exit" << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if(!glfwInit())
        return -1;

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 


  GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 

	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback        (window, OnKeyboardPressed);  
	glfwSetCursorPosCallback  (window, OnMouseMove); 
  glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);
	glfwSetInputMode          (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "../shaders/vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "../shaders/lambert.frag";
	ShaderProgram lambert(shaders); GL_CHECK_ERRORS;

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  GLuint vaoTriangle;
  GLsizei triangleIndices = CreateTriangle(vaoTriangle);

  GLuint vaoSphere;
  float radius = 1.0f;
  GLsizei sphereIndices = CreateSphere(radius, 8, vaoSphere);

  //ADDED
  GLuint vaoCuboid;
  GLsizei cuboidIndices = CreateCuboid(vaoCuboid, float4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 2.0f, 1.0f);

  GLuint vaoPlane;
  GLsizei planeIndices = CreatePlane(vaoPlane, float4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);

  GLuint vaoCone;
  GLsizei coneIndices = CreateCone(vaoCone, 1.0f, 6, 2.0f);

  GLuint vaoCylinder;
  GLsizei cylinderIndices = CreateCylinder(vaoCylinder, 1.0f, 6, 2.0f);

  bool direction = true;
  float3 downLimit = float3(-0.5f, 9.0f, -1.0f);
  float3 upLimit = float3(-0.5f, 10.0f, -1.0f);
  float3 currentPosition = downLimit;
  float mistake = 0.01f;

  glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
  glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
		//считаем сколько времени прошло за кадр
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
    doCameraMovement(camera, deltaTime);

		//очищаем экран каждый кадр
		glClearColor(0.9f, 0.95f, 0.97f, 1.0f); GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

    lambert.StartUseShader(); GL_CHECK_ERRORS;

    float4x4 view       = camera.GetViewMatrix();
    float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);
	  float4x4 model; 

    lambert.SetUniform("view", view);       GL_CHECK_ERRORS;
    lambert.SetUniform("projection", projection); GL_CHECK_ERRORS;

    glBindVertexArray(vaoSphere);
    {
      model = transpose(translate4x4(currentPosition));
      lambert.SetUniform("model", model); GL_CHECK_ERRORS;
      glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
	  if (direction) {
		  currentPosition.y += 0.01f;
	  }
	  else {
		  currentPosition.y -= 0.01f;
	  }

	  if (currentPosition.y >= upLimit.y  || currentPosition.y <= downLimit.y)
		  direction = !direction;
    }
    glBindVertexArray(0); GL_CHECK_ERRORS;

    glBindVertexArray(vaoTriangle); GL_CHECK_ERRORS;
    {
      model = transpose(translate4x4(float3(-0.5f, 6.0f, -1.0f)));
      lambert.SetUniform("model", model); GL_CHECK_ERRORS;
      glDrawElements(GL_TRIANGLE_STRIP, triangleIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
    }
    glBindVertexArray(0); GL_CHECK_ERRORS;

    glBindVertexArray(vaoCuboid); GL_CHECK_ERRORS;
    {
        model = transpose(translate4x4(float3(-1.0f, 3.0f, -1.5f)));
        lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLE_STRIP, cuboidIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
		
		float3 prevPosition = float3(-1.0f, 3.0f, -5.0f);
		float3 nextPosition = prevPosition;
		float angle = 0.0f;
		for (int i = 0; i < 80; i++) {
			model = transpose(mul(mul(translate4x4(nextPosition), rotate_Y_4x4(angle)), scale4x4(float3(0.5f, 0.07f, 1.f))));
			lambert.SetUniform("model", model); GL_CHECK_ERRORS;
			glDrawElements(GL_TRIANGLE_STRIP, cuboidIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			
			nextPosition.z = prevPosition.z * cosf(0.0009066f) + (-1 * sinf(0.0009066f) * prevPosition.x);
			nextPosition.x = prevPosition.z * sinf(0.0009066f) + (cosf(0.0009066f) * prevPosition.x);
			nextPosition.y+= 0.25f;
			prevPosition = nextPosition;
			angle += 180.0f;
		}
    }
    glBindVertexArray(0); GL_CHECK_ERRORS;

    glBindVertexArray(vaoPlane); GL_CHECK_ERRORS;
    {
        model = transpose(translate4x4(float3(-1.0f, 5.0f, -1.0f)));
        lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLE_STRIP, planeIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
    }
    glBindVertexArray(0); GL_CHECK_ERRORS; 

    glBindVertexArray(vaoCylinder); GL_CHECK_ERRORS;
    {
        model = transpose(translate4x4(float3(-0.5f, 1.0f, -1.0f)));
        lambert.SetUniform("model", model); GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLE_STRIP, cylinderIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
    }
    glBindVertexArray(0); GL_CHECK_ERRORS;
    //END ADDED

    lambert.StopUseShader(); GL_CHECK_ERRORS;

		glfwSwapBuffers(window); 
	}

  glDeleteVertexArrays(1, &vaoSphere);
  glDeleteVertexArrays(1, &vaoTriangle);
  //ADDED
  glDeleteVertexArrays(1, &vaoCuboid);
  glDeleteVertexArrays(1, &vaoPlane);
  glDeleteVertexArrays(1, &vaoCylinder);

  //END ADDED

	glfwTerminate();
	return 0;
}
