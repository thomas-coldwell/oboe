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

package com.google.oboe.samples.hellooboe;

import static android.app.PendingIntent.getActivity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.SimpleAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.google.oboe.samples.audio_device.AudioDeviceListEntry;
import com.google.oboe.samples.audio_device.AudioDeviceSpinner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

public class MainActivity extends Activity {
    private static final String TAG = "HelloOboe";
    private final ExecutorService audioThread = Executors.newSingleThreadExecutor();

    int outputDeviceId;
    int inputDeviceId;

    boolean didPress = false;
    final Handler handler = new Handler(Looper.getMainLooper());

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (didPress) {
            return true;
        }
        didPress = true;
        audioThread.execute(PlaybackEngine::runLoopbackTest);
        handler.postDelayed(() -> didPress = false, 1000);
        return true;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, 1);
        setContentView(R.layout.activity_main);
        audioThread.execute(() -> PlaybackEngine.startEngine(outputDeviceId, inputDeviceId));
    }

    @Override
    protected void onDestroy() {
        audioThread.execute(PlaybackEngine::stopEngine);
        super.onDestroy();
    }
}
