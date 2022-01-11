package com.youngpark.simplechinesechess.Activity;

import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.youngpark.simplechinesechess.Engine.GameEngine;
import com.youngpark.simplechinesechess.R;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    private void setGame() {
        final Button pvpButton, pveButton, eveButton, customButton;
        final EditText thinkDepthEditText, thinkTimeEditText;

        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(MainActivity.this);

        View dialogView = View.inflate(MainActivity.this, R.layout.dialog_choose, null);

        pvpButton = dialogView.findViewById(R.id.pvpButton);
        pveButton = dialogView.findViewById(R.id.pveButton);
        eveButton = dialogView.findViewById(R.id.eveButton);
        customButton = dialogView.findViewById(R.id.customButton);

        thinkDepthEditText = dialogView.findViewById(R.id.thinkDepthEditText);
        thinkTimeEditText = dialogView.findViewById(R.id.thinkTimeEditText);

        Button.OnClickListener clickListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                    case R.id.pvpButton:
                        GameEngine.GameMode = 0;
                        Toast.makeText(MainActivity.this, "已选PvP", Toast.LENGTH_SHORT).show();
                        break;
                    case R.id.pveButton:
                        GameEngine.GameMode = 1;
                        Toast.makeText(MainActivity.this, "已选PvE", Toast.LENGTH_SHORT).show();
                        break;
                    case R.id.eveButton:
                        GameEngine.GameMode = 2;
                        Toast.makeText(MainActivity.this, "已选EvE", Toast.LENGTH_SHORT).show();
                        break;
                    case R.id.customButton:

                        Toast.makeText(MainActivity.this, "待更新", Toast.LENGTH_SHORT).show();
                        break;
                }
            }
        };

        pvpButton.setOnClickListener(clickListener);
        pveButton.setOnClickListener(clickListener);
        eveButton.setOnClickListener(clickListener);
        customButton.setOnClickListener(clickListener);

        dialogBuilder.setView(dialogView);

        dialogBuilder.setTitle("选择游戏类型");
        dialogBuilder.setIcon(getDrawable(R.drawable.icon));
        dialogBuilder.setPositiveButton("确定", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent = new Intent(MainActivity.this, GameActivity.class);
                startActivity(intent);
            }
        });
        dialogBuilder.setNegativeButton("取消", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        dialogBuilder.create().show();
    }

    public void startGame(View view) {
        setGame();
    }

    public void showRank(View view) {
        Intent intent = new Intent(MainActivity.this, RankActivity.class);
        startActivity(intent);
    }
}
