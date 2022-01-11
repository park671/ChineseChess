package com.youngpark.simplechinesechess.Activity;

import android.graphics.Color;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.youngpark.simplechinesechess.Engine.GameEngine;
import com.youngpark.simplechinesechess.Engine.SoundEngine;
import com.youngpark.simplechinesechess.NativeBridge.AIBridge;
import com.youngpark.simplechinesechess.R;
import com.youngpark.simplechinesechess.SituationSetter;
import com.youngpark.simplechinesechess.View.GameView;

import java.text.DecimalFormat;

public class GameActivity extends AppCompatActivity implements SituationSetter {

    private TextView roundTextView, situationTextView, logTextView, gameModeTextView;
    private GameView gameView;
    private SoundEngine soundEngine;
    private Button undoButton, newRoundButton;

    private ProgressBar AiProgressBar;

    Thread drawThread;
    Handler handler = new Handler(Looper.getMainLooper());

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        GameEngine.init();
        initSoundEngine();

        setContentView(R.layout.activity_game);
        roundTextView = findViewById(R.id.WhoTextView);
        logTextView = findViewById(R.id.LogTextView);
        situationTextView = findViewById(R.id.SituationTextView);
        gameView = findViewById(R.id.GameView);
        AiProgressBar = findViewById(R.id.AiProgressBar);
        undoButton = findViewById(R.id.undoButton);
        newRoundButton = findViewById(R.id.newRound);
        gameModeTextView = findViewById(R.id.GameModeTextView);

        gameView.setRoundSetter(this);
    }

    private double runningTime = 0;
    private DecimalFormat decimalFormat = new DecimalFormat("0.0");

    @Override
    protected void onResume() {
        super.onResume();
        drawThread = new Thread() {
            @Override
            public void run() {
                while (true) {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            gameView.invalidate();
                            logTextView.setText("已用时间" + decimalFormat.format(runningTime) + "s");
                        }
                    });
                    runningTime += 0.1;
                    try {
                        Thread.sleep(100);
                    } catch (Throwable tr) {
                        tr.printStackTrace();
                    }
                }
            }
        };
        drawThread.start();

        switch (GameEngine.GameMode) {
            case 0:
                gameModeTextView.setText("玩家 vs 玩家");
                break;
            case 1:
                gameModeTextView.setText("玩家 vs 电脑");
                break;
            case 2:
                gameModeTextView.setText("电脑 vs 电脑");
                break;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        try {
            drawThread.stop();
        } catch (Throwable tr) {
            tr.printStackTrace();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        GameEngine.over = 3;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        soundEngine.stop();
    }

    private void initSoundEngine() {
        soundEngine = new SoundEngine();
        soundEngine.init(this);
        soundEngine.start();
    }

    @Override
    public void setRound(boolean red) {
        runningTime = 0;
        if (red) {
            roundTextView.setText("红 方 (" + (GameEngine.GameMode != 2 ? "玩家 1" : "AI 1") + ")");
            AiProgressBar.setVisibility((GameEngine.GameMode != 2 ? View.INVISIBLE : View.VISIBLE));
            undoButton.setEnabled(GameEngine.GameMode != 2 ? true : false);
            newRoundButton.setEnabled(GameEngine.GameMode != 2 ? true : false);
            roundTextView.setBackgroundColor(getResources().getColor(R.color.red));
        } else {
            roundTextView.setText("黑 方 (" + (GameEngine.GameMode != 0 ? "AI 2" : "玩家 2") + ")");
            AiProgressBar.setVisibility((GameEngine.GameMode != 0 ? View.VISIBLE : View.INVISIBLE));
            undoButton.setEnabled(false);
            newRoundButton.setEnabled(false);
            roundTextView.setBackgroundColor(getResources().getColor(R.color.black));
        }
    }

    public void startNew(View view) {
        GameEngine.init();
        gameView.invalidate();
    }

    public void backUp(View view) {
        GameEngine.backUp();
        gameView.invalidate();
    }

    @Override
    public void setSituation(int redVal, int blackVal) {
        situationTextView.setText("红方局势:" + redVal + ", 黑方局势:" + blackVal);
    }

    @Override
    public void setOver(boolean win) {
        roundTextView.setText(win ? "你赢了！" : "你输了。");
        roundTextView.setBackgroundColor(win ? Color.GREEN : Color.GRAY);
    }

    public void test(View view) {
        String msg = AIBridge.test("youngpark's msg");
        Toast.makeText(GameActivity.this, msg, Toast.LENGTH_SHORT).show();
    }
}
