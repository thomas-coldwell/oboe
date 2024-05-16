/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>
#include <oboe/Oboe.h>
#include "HelloOboeEngine.h"
#include "logging_macros.h"

extern "C" {
/* We only need one HelloOboeEngine and it is needed for the entire time.
 * So just allocate one statically. */
static HelloOboeEngine sEngine;

JNIEXPORT jint JNICALL
Java_com_google_oboe_samples_hellooboe_PlaybackEngine_startEngine(
        JNIEnv *env,
        jclass,
        jint outputDeviceId, jint inputDeviceId, jstring filesDir) {
    const char *path = env->GetStringUTFChars(filesDir, NULL);
    sEngine.filesDir = std::string(path);
    return static_cast<jint>(sEngine.start(outputDeviceId, inputDeviceId));
}

JNIEXPORT jint JNICALL
Java_com_google_oboe_samples_hellooboe_PlaybackEngine_stopEngine(
        JNIEnv *env,
        jclass) {
    return static_cast<jint>(sEngine.stop());
}


JNIEXPORT void JNICALL
Java_com_google_oboe_samples_hellooboe_PlaybackEngine_runLoopbackTest(JNIEnv *env, jclass clazz) {
    return sEngine.runLoopbackTest();
}
} // extern "C"