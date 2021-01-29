//
// Created by gacmy on 2021/1/29.
//

#include "test_crash_jni.h"
#include <jni.h>

void foo() {
  volatile int* p = nullptr;
  *p = 1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_test_crash_TestCrash_initNative(JNIEnv* env, jobject clazz) {
   foo();
}

