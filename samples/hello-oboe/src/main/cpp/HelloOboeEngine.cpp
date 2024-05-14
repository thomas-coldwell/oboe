/**
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


#include <inttypes.h>
#include <memory>
#include <thread>
#include <chrono>
#include <android/log.h>

#include "HelloOboeEngine.h"

double now_ms() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

HelloOboeEngine::HelloOboeEngine() {
}

void HelloOboeEngine::runLoopbackTest() {
    thresholdTime = now_ms() + 500.0;
    inputRecordThread = std::thread([=]() {
        unlink("/storage/emulated/0/Download/tom/oboe.pcm");
        FILE *debugFile = fopen("/storage/emulated/0/Download/tom/oboe.pcm", "wb");
        int framesRead = 0;
        while (true) {
            auto buffer = new int16_t[256];
            memset(buffer, 0, 256 * 2);
            mInputStream->read(buffer, 256, 10'000'000).value();
            fwrite(buffer, sizeof(int16_t), 256, debugFile);
            framesRead += 256;
            delete[] buffer;
            if (framesRead > 44100 * 2) {
                break;
            }
        }
        fclose(debugFile);
    });
    inputRecordThread.join();
    __android_log_print(
            ANDROID_LOG_INFO,
            "TOM",
            "Output: %f %lld Input: %f %lld",
            mOutputStream->calculateLatencyMillis().value(),
            mOutputStream->getFramesWritten(),
            mInputStream->calculateLatencyMillis().value(),
            mInputStream->getFramesRead()
            );
}

oboe::Result HelloOboeEngine::openPlaybackStream() {
    oboe::AudioStreamBuilder builder;
    outputDataCallback = std::make_shared<OutputDataCallback>(this);
    oboe::Result result = builder.setDirection(oboe::Direction::Output)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setAudioApi(oboe::AudioApi::AAudio)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setFormat(oboe::AudioFormat::Float)
        ->setChannelCount(oboe::ChannelCount::Stereo)
        ->setSampleRate(48000)
        ->setDeviceId(mOutputDeviceId)
        ->setDataCallback(outputDataCallback)
        ->openStream(mOutputStream);
    mOutputStream->setBufferSizeInFrames(mOutputStream->getFramesPerBurst());
    return result;
}

oboe::Result HelloOboeEngine::openRecordingStream() {
    oboe::AudioStreamBuilder builder;
    oboe::Result result = builder.setDirection(oboe::Direction::Input)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setAudioApi(oboe::AudioApi::AAudio)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setSampleRate(44100)
            ->setDeviceId(mInputDeviceId)
            ->setInputPreset(oboe::InputPreset::Generic)
            ->setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Fastest) // SUPER IMPORTANT TO BE SET!!! THIS IS NOT INCLUDED IN LATENCY MEASUREMENTS BUT ADDS SIGNIFICANT PROCESSING TIME!!!
            ->openStream(mInputStream);
    return result;
}

oboe::Result HelloOboeEngine::start(int outputDeviceId, int inputDeviceId) {
    mOutputDeviceId = outputDeviceId;
    mInputDeviceId = inputDeviceId;
    return start();
}

oboe::Result HelloOboeEngine::start() {
    std::lock_guard<std::mutex> lock(mLock);
    openPlaybackStream();
    mOutputStream->requestStart();
    openRecordingStream();
    mInputStream->requestStart();
    return oboe::Result::OK;
}

oboe::Result HelloOboeEngine::stop() {
    // Stop, close and delete in case not already closed.
    std::lock_guard<std::mutex> lock(mLock);
    if (mOutputStream) {
        mOutputStream->stop();
        mOutputStream->close();
        mOutputStream.reset();
    }
    if (mInputStream) {
        mInputStream->stop();
        mInputStream->close();
        mInputStream.reset();
    }
    return oboe::Result::OK;
}

HelloOboeEngine::OutputDataCallback::OutputDataCallback(HelloOboeEngine *engine) {
    this->mEngine = engine;
}

oboe::DataCallbackResult
HelloOboeEngine::OutputDataCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData,
                                                  int32_t numFrames) {
    float *buffer = static_cast<float*>(audioData);
    auto now = (double) (audioStream->getTimestamp(CLOCK_MONOTONIC).value().timestamp / 1'000'000LL);
    auto thresholdTimeDeltaMs = now - mEngine->thresholdTime;
    for (int i = 0; i < numFrames; i++) {
        auto relativeSampleTimeDeltaMs = (int64_t) (thresholdTimeDeltaMs + ((double) i / 48000.0) * 1000.0);
        if (relativeSampleTimeDeltaMs > 0.0 && relativeSampleTimeDeltaMs < 1000.0) {
            float noise = (float) ((drand48() - 0.5) * 0.6);
            buffer[i * 2] = noise;
            buffer[i * 2 + 1] = noise;
        }
        else {
            buffer[i * 2] = 0.0;
            buffer[i * 2 + 1] = 0.0;
        }
    }
    return oboe::DataCallbackResult::Continue;
}
