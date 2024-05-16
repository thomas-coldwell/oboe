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


import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
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

import java.io.File;
import java.net.URI;
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
        handler.postDelayed(() -> {
            didPress = false;
            File filesDirectory = getApplicationContext().getFilesDir();
            String path = filesDirectory.getPath() + "/test.pcm";
            File file = new File(path);
            Uri uri = FileProvider.getUriForFile(getApplicationContext(), BuildConfig.APPLICATION_ID + ".provider", file);
            Intent sendIntent = new Intent();
            sendIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            sendIntent.setAction(Intent.ACTION_SEND);
            sendIntent.putExtra(Intent.EXTRA_STREAM, uri);
            sendIntent.setType("audio/*");
            startActivity(Intent.createChooser(sendIntent, null));
        }, 2500);
        return true;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, 1);
        setContentView(R.layout.activity_main);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
            AudioDeviceInfo[] inputDevices = audioManager.getDevices(AudioManager.GET_DEVICES_INPUTS);
            for (AudioDeviceInfo device : inputDevices) {
                if (inputDeviceId == 0 && device.getType() == AudioDeviceInfo.TYPE_BUILTIN_MIC) {
                    Log.i("TOM", String.valueOf(device.getId()));
                    inputDeviceId = device.getId();
                }
            }
        }

        File filesDirectory = getApplicationContext().getFilesDir();

        audioThread.execute(() -> PlaybackEngine.startEngine(outputDeviceId, inputDeviceId, filesDirectory.getPath()));
    }

    @Override
    protected void onDestroy() {
        audioThread.execute(PlaybackEngine::stopEngine);
        super.onDestroy();
    }
}
