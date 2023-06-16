#ifndef ANDROID2IOS_H
#define ANDROID2IOS_H

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "include/EGL/egl.h"

typedef struct ANativeWindow {
    EGLNativeWindowType window;
       EGLDisplay display;
       EGLSurface surface;
       EGLContext context;
} ANativeWindow;


typedef struct AConfiguration {
    GLint viewportWidth;
    GLint viewportHeight;
    GLfloat zNear;
    GLfloat zFar;
} AConfiguration;

typedef struct AInputEvent {
    int type;
    int action;
    int x;
    int y;
} AInputEvent;

typedef struct android_app {
   // struct android_poll_source* cmdPollSource;
   // struct android_poll_source* inputPollSource;
    EGLNativeWindowType window;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    int32_t cmdExit;
    int32_t inputExit;
} android_app;

#endif // DEVICE_TOOLS_AIDL_AIDL_LANGUAGE_H
