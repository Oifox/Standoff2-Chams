//
// Created by Admin on 03.06.2020.
//

#ifndef SO2HACK_CHAMS_H
#define SO2HACK_CHAMS_H
#include <GLES2/gl2.h>
#include <dlfcn.h>


static void *handle;
static const char* shaderName;
static bool enableWallhack;
static int r, g, b = 5;

void setShader(const char* s) {
    shaderName = s;
}
const char* getShader() {
    return shaderName;
}

void SetWallhack(bool enable){
    enableWallhack = enable;
}

void SetColor(int p1, int p2, int p3){
    r = p1;
    g = p2;
    b = p3;
}

bool getWallhackEnabled(){
    return enableWallhack;
}


int (*old_glGetUniformLocation)(GLuint, const GLchar *);
GLint new_glGetUniformLocation(GLuint program, const GLchar *name) {
    //Log name to find used shaders.
    LOGI("New shader found: %s", name);
    return old_glGetUniformLocation(program, name);
}

bool isCurrentShader(const char *shader) {
    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    return old_glGetUniformLocation(currProgram, shader) != -1;
}

void (*old_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
void new_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
    old_glDrawElements(mode, count, type, indices);
    if (mode != GL_TRIANGLES || count < 1000) return;
    {
        GLint currProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

        GLint id = old_glGetUniformLocation(currProgram, getShader());
        if (id == -1) return;

        if (getWallhackEnabled()) {

            glDepthRangef(1, 0.5);

            glColorMask(r, g, b, 0);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        }

        old_glDrawElements(mode, count, type, indices);


        glDepthRangef(0.5, 1);

        glColorMask(1, 1, 1, 1);
        glDisable(GL_BLEND);
    }
}

bool mlovinit(){
    handle = NULL;
    handle = dlopen("libGLESv2.so", RTLD_LAZY);
    if(!handle){
        LOGE("Cannot open library: %s", dlerror());
        return false;
    }
    return true;
}

void LogShaders(){
    auto p_glGetUniformLocation = (const void*(*)(...))dlsym(handle, "glGetUniformLocation");
    const char *dlsym_error = dlerror();
    if(dlsym_error){
        LOGE("Cannot load symbol 'glGetUniformLocation': %s", dlsym_error);
        return;
    }else{
 MSHookFunction(reinterpret_cast<void*>(p_glGetUniformLocation), reinterpret_cast<void*>(new_glGetUniformLocation), reinterpret_cast<void**>(&old_glGetUniformLocation));
    }
}

void Wallhack(){
    auto p_glDrawElements = (const void*(*)(...))dlsym(handle, "glDrawElements");
    const char *dlsym_error = dlerror();
    if(dlsym_error){
        LOGE("Cannot load symbol 'glDrawElements': %s", dlsym_error);
        return;
    }else{
        MSHookFunction(reinterpret_cast<void*>(p_glDrawElements), reinterpret_cast<void*>(new_glDrawElements), reinterpret_cast<void**>(&old_glDrawElements));
    }
}

#endif //SO2HACK_CHAMS_H
