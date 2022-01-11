package com.youngpark.simplechinesechess.Engine;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.SoundPool;

import com.youngpark.simplechinesechess.R;

public class SoundEngine {

    private SoundPool soundPool;

    public int selectSound, moveSound, winSound, startSound;

    public void init(Context context) {
        AudioAttributes audioAttributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build();

        soundPool = new SoundPool.Builder().setMaxStreams(3)
                .setAudioAttributes(audioAttributes)
                .build();
        selectSound = soundPool.load(context, R.raw.marking, 1);
        moveSound = soundPool.load(context, R.raw.check, 1);
        winSound = soundPool.load(context, R.raw.win, 1);
        startSound = soundPool.load(context, R.raw.start, 1);
    }

    public void play(int soundId) {
        soundPool.play(soundId, 1, 1, 1, 0, 1);
    }

    private volatile boolean soundEngineStarted = true;

    public void start() {
        Thread soundThread = new Thread() {
            @Override
            public void run() {
                while (soundEngineStarted) {
                    while (GameEngine.selectSoundCount > 0) {
                        play(selectSound);
                        GameEngine.selectSoundCount--;
                    }
                    while (GameEngine.moveSoundCount > 0) {
                        play(moveSound);
                        GameEngine.moveSoundCount--;
                    }
                    while (GameEngine.startSoundCount > 0) {
                        play(startSound);
                        GameEngine.startSoundCount--;
                    }
                }
            }
        };
        soundThread.start();
    }

    public void stop() {
        soundEngineStarted = false;
    }

}
