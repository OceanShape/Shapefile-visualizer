#pragma warning(disable:4996)
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <iomanip>
#include <vector>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shapedata.h"

using namespace std;

typedef unsigned char uchar;

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;

GLuint vbo;
GLuint program;

vector<float> vertices;
vector<int> objectVertices;

bool checkShaderCompileStatus(GLuint shader)
{
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        vector<char> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog.data());
        cerr << "Shader compile error: " << infoLog.data() << endl;
        return false;
    }
    std::cout << "Shader compile complite" << endl;
    return true;
}

bool compileShader(GLuint shader, const char* source)
{
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return checkShaderCompileStatus(shader);
}

string readShader(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }

    string shader_code;
    string line;
    while (getline(file, line)) {
        shader_code += line + "\n";
    }

    return shader_code;
}

bool initialize()
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig eglConfig;
    EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
    EGL_NONE
    };
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, &majorVersion, &minorVersion);
    eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &numConfigs);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, hWnd, NULL);
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);


    string shaderSource = readShader("source.vert");
    const char* shaderCstr = shaderSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertexShader, shaderCstr)) {
        return false;
    }

    shaderSource = readShader("source.frag");
    shaderCstr = shaderSource.c_str();

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragmentShader, shaderCstr)) {
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    return true;
}

void render()
{
    glm::vec3 cameraPosition = glm::vec3(cameraX, cameraY, 1.0f);
    glm::vec3 cameraTarget = glm::vec3(cameraX, cameraY, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0, startIndex = 0; i < objectVertices.size(); ++i) {
        glDrawArrays(GL_LINE_STRIP, startIndex, objectVertices[i]);
        startIndex += objectVertices[i];
    }
}


void cleanUp()
{
    glDeleteBuffers(1, &vbo);

    GLuint vertexShader;
    glGetAttachedShaders(program, 1, NULL, &vertexShader);
    GLuint fragmentShader;
    glGetAttachedShaders(program, 1, NULL, &fragmentShader);
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);

    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
}

SHPHeader header;
SHPRecordData record;

void memSwap(void* const data, size_t size) {
    uint8_t* start = (uint8_t*)data;
    uint8_t* end = (uint8_t*)data + size - 1;
    while (start < end) {
        uint8_t tmp = *start;
        *start = *end;
        *end = tmp;
        start++, end--;
    }
}

bool readShapefile(const char* fileName) {
    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) {
        std::cout << "Failed to open file\n";
        return false;
    }
                 
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    // Shapefile은 한 파일에 하나의 SHP 타입만 존재
    // 즉, 헤더 분석 과정에서 타입 특정 가능

    uchar* data = new uchar[fileSize];
    memset(data, 0, fileSize);
    fread(data, sizeof(uchar), fileSize, fp);

    uchar* offset = data;
    
    SHPHeader shpHeaderData;
    /*Byte 0     File Code    9994        Integer Big
    Byte 4     Unused       0           Integer Big
    Byte 8     Unused       0           Integer Big
    Byte 12    Unused       0           Integer Big
    Byte 16    Unused       0           Integer Big
    Byte 20    Unused       0           Integer Big
    Byte 24    File Length  File Length Integer Big
    Byte 28    Version      1000        Integer Little
    Byte 32    Shape Type   Shape Type  Integer Little
    Byte 36    Bounding Box Xmin        Double  Little
    Byte 44    Bounding Box Ymin        Double  Little
    Byte 52    Bounding Box Xmax        Double  Little
    Byte 60    Bounding Box Ymax        Double  Little
    Byte 68 * Bounding Box Zmin        Double  Little
    Byte 76 * Bounding Box Zmax        Double  Little
    Byte 84 * Bounding Box Mmin        Double  Little
    Byte 92 * Bounding Box Mmax        Double  Little*/

    // File Code
    std::memcpy(&shpHeaderData.fileCode, offset, 4); offset += 4;
    memSwap(&shpHeaderData.fileCode, 4);

    // Unused
    offset += 20;

    // File Length
    std::memcpy(&shpHeaderData.fileLen, offset, 4);  offset += 4;
    memSwap(&shpHeaderData.fileLen, 4);

    // version
    std::memcpy(&shpHeaderData.version, offset, 4);  offset += 4;

    // Shape Type
    std::memcpy(&shpHeaderData.SHPType, offset, 4);  offset += 4;
    
    // Bounding Box
    std::memcpy(&shpHeaderData.Xmin, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Ymin, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Xmax, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Ymax, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Zmin, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Zmax, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Mmin, offset, 8); offset += 8;
    std::memcpy(&shpHeaderData.Mmax, offset, 8); offset += 8;

    cout << (shpHeaderData.Xmax - shpHeaderData.Xmin) / (shpHeaderData.Ymax - shpHeaderData.Ymin) << endl;

    int32_t recordCount = 0;
    bool isPrintStatus = true;
    SHPPoint* points = new SHPPoint[1000];
    double* Zpoints = new double[1000];
    int32_t* parts = new int32_t[1000];

    //while (offset < data + fileSize) {
    for (size_t r = 0; r < 1; ++r) {
        uchar* startOffset = offset;

        bool hasZvalue = false;

        int32_t recordNum;
        int32_t contentLength;

        int32_t shapeType;
        double box[4];
        int32_t numParts;
        int32_t numPoints;
        double Zrange[2];   // min, max


        std::memcpy(&recordNum, offset, 4);  offset += 4;
        memSwap(&recordNum, 4);

        std::memcpy(&contentLength, offset, 4);  offset += 4;
        memSwap(&contentLength, 4);

        std::memcpy(&shapeType, offset, 4);  offset += 4;

        std::memcpy(box, offset, sizeof(double) * 4);  offset += sizeof(double) * 4;

        std::memcpy(&numParts, offset, 4);  offset += 4;
        std::memcpy(&numPoints, offset, 4);  offset += 4;

        std::memcpy(parts, offset, sizeof(int32_t) * numParts);	offset += sizeof(int32_t) * numParts;

        std::memcpy(points, offset, sizeof(SHPPoint) * numPoints);	offset += sizeof(SHPPoint) * numPoints;


        // Z point
        if (offset - startOffset < contentLength * 2) {
            hasZvalue = true;

            std::memcpy(Zrange, offset, sizeof(double) * 2);    offset += sizeof(double) * 2;
            std::memcpy(Zpoints, offset, sizeof(double) * numPoints);	offset += sizeof(double) * numPoints;
        }

        // M point(ignore)
        if (offset - startOffset < contentLength * 2) {
            //double Mrange[2];
            //float Mpoints[1000];
            //std::memcpy(Mrange, offset, sizeof(double) * 2);
            offset += sizeof(double) * 2;
            //std::memcpy(Mpoints, offset, sizeof(double) * numPoints);
            offset += sizeof(double) * numPoints;
        }

        if(isPrintStatus) {
            std::printf("recordNum:\t%d\n", recordNum);
            std::printf("contentLength:\t%d\n", contentLength);
            std::printf("shapeType:\t%d\n", shapeType);

            std::printf("bounding\n");
            for (size_t j = 0; j < 4; ++j) {
                std::printf("\t%f\n", box[j]);
            }
            std::printf("numParts:\t%d\n", numParts);
            for (size_t p = 0; p < numParts; ++p) {
                std::printf("\t%d\n", parts[p]);
            }

            std::printf("numPoints:\t%d\n", numPoints);
            if (hasZvalue) {
                for (size_t p = 0; p < numPoints; ++p) {
                    std::printf("\t%0.16f, %0.16f, %0.16f\n", points[p].x, points[p].y, Zpoints[p]);
                }
                std::printf("Z range: %0.16f, %0.16f\n", Zrange[0], Zrange[1]);
            }
            else {
                for (size_t p = 0; p < numPoints; ++p) {
                    std::printf("\t%0.16f, %0.16f\n", points[p].x, points[p].y);
                }
            }

            std::cout << "---" << endl;
        }

        recordCount++;
        //std::cout << recordNum << endl;
    }


    std::cout << "total record count: " << recordCount << endl;

    std::fclose(fp);
    delete[] Zpoints;
    delete[] parts;
    delete[] points;
    delete[] data;

    return true;
}



int main() {
    readShapefile("B3_SURFACEMARK.shp");
    return 0;
}
