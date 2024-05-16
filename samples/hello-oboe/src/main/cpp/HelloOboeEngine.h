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

#ifndef OBOE_HELLO_OBOE_ENGINE_H
#define OBOE_HELLO_OBOE_ENGINE_H

#include <oboe/Oboe.h>
#include <thread>

constexpr int32_t kBufferSizeAutomatic = 0;

class HelloOboeEngine {

public:
    HelloOboeEngine();
    oboe::Result start(int outputDeviceId, int inputDeviceId);
    oboe::Result start();
    oboe::Result stop();
    void runLoopbackTest();

public:
    std::atomic<double> thresholdTime = 0.0;
    std::string filesDir;

private:
    class OutputDataCallback: public oboe::AudioStreamDataCallback {
    public:
        OutputDataCallback(HelloOboeEngine *engine);
        oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;

    private:
        HelloOboeEngine *mEngine;
    };

    std::shared_ptr<OutputDataCallback> outputDataCallback;
    std::thread inputRecordThread;

    oboe::Result openPlaybackStream();
    oboe::Result openRecordingStream();

    std::shared_ptr<oboe::AudioStream> mOutputStream;
    std::shared_ptr<oboe::AudioStream> mInputStream;

    int32_t mOutputDeviceId = oboe::Unspecified;
    int32_t mInputDeviceId = oboe::Unspecified;
    std::mutex mLock;
};

#endif //OBOE_HELLO_OBOE_ENGINE_H
