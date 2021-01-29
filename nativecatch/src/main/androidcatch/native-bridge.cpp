#include <jni.h>
#include "native-bridge.h"
int g_android_api = 17;

static JavaVM *g_currentJVM = nullptr;
static jclass g_cls = nullptr;

static int registerNativeMethods(JNIEnv *env,jclass cls);

extern "C" JNIEXPORT JNICALL jint JNI_OnLoad(JavaVM* vm,void* reserved){
  g_currentJVM = vm;
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
      return -1;
  }
  if(g_cls){
    env->DeleteGlobalRef(g_cls);
  }
  static const char* clsName = "com/gac/nativelib/NativeCatch"; 
  jclass instance = env->FindClass(clsName);
  if(!instance){
    LOGE("fail to create class: %s",clsName); 
    return -2;
  }
  g_cls = reinterpret_cast<jclass>(env->NewGlobalRef(instance));
  if(!g_cls){
    LOGE("Failed to create global reference: %s",clsName);
    return -3;
  } 
  int ret = registerNativeMethods(env, g_cls);
  if(ret != 0){
    LOGE("fail to register methods for class:%s ret:%d ",clsName,ret);
    return -4;
  }

  jclass versionClass = env->FindClass("android/os/Build$VERSION");
    if (versionClass) {
        jfieldID sdkIntFieldID = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
        if (sdkIntFieldID) {
            g_android_api = env->GetStaticIntField(versionClass, sdkIntFieldID);
           LOGI("current API level = %d", g_android_api);
        } else {
            LOGE("fail to get field id android.os.Build.VERSION.SDK_INT");
        }
    } else {
        LOGE("fail to get class android.os.Build.VERSION");
    }

    return JNI_VERSION_1_6;
}

static JNIEnv *getCurrentEnv() {
    if (g_currentJVM) {
        JNIEnv *currentEnv = nullptr;
        auto ret = g_currentJVM->GetEnv(reinterpret_cast<void **>(&currentEnv), JNI_VERSION_1_6);
        if (ret == JNI_OK) {
            return currentEnv;
        } else {
            LOGE("fail to get current JNIEnv: %d", ret);
        }
    }
    return nullptr;
}

#define CATCH_JNI static

namespace CATCH{
  CATCH_JNI void init(JNIEnv *env, jobject instance) {
    LOGI("init catch");
    catch_handler_setup(1);    
  }

};

static JNINativeMethod g_method[] = {
  {"init","()V",(void*)CATCH::init},
};

static int registerNativeMethods(JNIEnv *env,jclass cls){
  return env->RegisterNatives(cls, g_method, sizeof(g_method)/sizeof(g_method[0]));
}



