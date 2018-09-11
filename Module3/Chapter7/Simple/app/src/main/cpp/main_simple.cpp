#include <jni.h>

#include <GLES3/gl3.h>

#include <android/log.h>
#include <array>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>

constexpr std::size_t MAX_INFO_LENGTH = 512;

#define LOG_TAG "libgl3jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


struct Common {
    GLuint mProgram;
    GLint mvPositionHandle;
    GLint mvColorHandle;

    static constexpr int WIDTH  = 1280;
    static constexpr int HEIGHT = 720;

    static const std::string V_SHADER;
    static const std::string F_SHADER;

    static const GLfloat TRIANGLE[9];
    static const GLfloat COLOR[9];
};
static Common *g_pCommon = nullptr;

constexpr GLfloat Common::TRIANGLE[9] =
        {-1.0f, -1.0f, 0.0f,
          1.0f, -1.0f, 0.0f,
          0.0f,  1.0f, 0.0f };
constexpr GLfloat Common::COLOR[9] =
        { 1.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 1.0f };

const std::string Common::V_SHADER = {
"#version 300 es\n"
"in vec4 vPosition;\n"
"in vec4 vColor;\n"
"out vec4 color;\n"
"void main() {\n"
"\tgl_Position = vPosition;\n"
"\tcolor       = vColor;\n"
"}\n"
};
const std::string Common::F_SHADER  = {
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 color;\n"
        "out vec4 color_out;\n"
        "void main() {\n"
        "\tcolor_out = color;\n"
        "}\n"
};

static void printGLString(const char *name, GLenum value) {
    const char *str = reinterpret_cast<const char*>(glGetString(value));
    LOGI("GL %s = %s\n", name, str);
}

static void checkGLError(const char *name) {
    for(GLint error = glGetError(); error; error = glGetError()) {
        LOGI("After %s() glError (0x%x)\n", name, error);
    }
}

static GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if(shader) {
        glShaderSource(shader, 1, &pSource, 0);
        glCompileShader(shader);

        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        //Report error and delete the shader
        if(!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen > MAX_INFO_LENGTH - 1) {
                std::string buffer;
                buffer.resize(infoLen);
                glGetShaderInfoLog(shader, infoLen, 0, &buffer[0]);
                LOGE("Could not compile shader %d:\\n%s\\n", shaderType, buffer.data());
            } else {
                std::array<char, MAX_INFO_LENGTH> buffer;
                glGetShaderInfoLog(shader, infoLen, 0, &buffer[0]);
                buffer[infoLen] = 0;
                LOGE("Could not compile shader %d:\\n%s\\n", shaderType, buffer.data());
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

static GLuint createShaderProgram(const char *vertexShaderCode,
                                  const char *fragmentShaderCode) {
    GLuint vertexShaderId   = loadShader(GL_VERTEX_SHADER,   vertexShaderCode);
    if(!vertexShaderId) {
        return 0;
    }
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
    if(!fragmentShaderId) {
        return 0;
    }
    GLint  result    = GL_FALSE;
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderId);
    checkGLError("glAttachShader");
    glAttachShader(programID, fragmentShaderId);
    checkGLError("glAttachShader");
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if(result != GL_TRUE) {
        GLint infoLen = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > MAX_INFO_LENGTH - 1) {
            std::string buffer;
            buffer.resize(infoLen);
            glGetProgramInfoLog(programID, infoLen, 0, &buffer[0]);
            LOGE("Could not link program:\n%s\n", buffer.data());
        } else {
            std::array<char, MAX_INFO_LENGTH> buffer;
            glGetProgramInfoLog(programID, infoLen, 0, &buffer[0]);
            buffer[infoLen] = 0;
            LOGE("Could not link program:\n%s\n", buffer.data());
        }
        glDeleteProgram(programID);
        programID = 0;
    } else {
        LOGI("Linked program Successfully\n");
    }
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return programID;
}

static bool setupGraphics(int width, int height) {
    printGLString("Version   ", GL_VERSION);
    printGLString("Vendor    ", GL_VENDOR);
    printGLString("Renderer  ", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", width, height);
    auto& program = g_pCommon->mProgram;
    program = createShaderProgram(Common::V_SHADER.c_str(), Common::F_SHADER.c_str());
    if(!program) {
        LOGE("Could not create program.");
        return false;
    }

    g_pCommon->mvPositionHandle = glGetAttribLocation(program, "vPosition");
    g_pCommon->mvColorHandle    = glGetAttribLocation(program, "vColor");

    glViewport(0, 0, width, height);
    return true;
}

void renderFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_pCommon->mProgram);

    GLuint pos   = static_cast<GLuint>(g_pCommon->mvPositionHandle);
    GLuint color = static_cast<GLuint>(g_pCommon->mvColorHandle);

    glVertexAttribPointer(pos,   3, GL_FLOAT, GL_FALSE, 0, Common::TRIANGLE);
    glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 0, Common::COLOR);

    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(color);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ahussein_simple_GL3JNILib_init(JNIEnv *env, jclass type, jint width, jint height) {
    g_pCommon = new Common;
    setupGraphics(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ahussein_simple_GL3JNILib_step(JNIEnv *env, jclass type) {
    renderFrame();
}