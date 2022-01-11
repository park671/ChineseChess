package com.youngpark.simplechinesechess.View;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.youngpark.simplechinesechess.Engine.AIEngine;
import com.youngpark.simplechinesechess.Engine.GameEngine;
import com.youngpark.simplechinesechess.Engine.GraphicEngine;
import com.youngpark.simplechinesechess.SituationSetter;

import java.lang.ref.WeakReference;

public class GameView extends View {
    public GameView(Context context) {
        super(context);
        init();
    }

    public GameView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public GameView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private GraphicEngine.Resource resource;

    private void init() {
        resource = new GraphicEngine.Resource();
        resource.init(this);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        resource.scaleBitmap(getWidth());
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int width = MeasureSpec.getSize(widthMeasureSpec);
        setMeasuredDimension(width, (int) (width * 1.11));
    }

    private Paint paint = new Paint();

    @Override
    protected void onDraw(Canvas canvas) {
        //drawBoard
        canvas.drawBitmap(resource.boardBmp, 0, 0, paint);
        //drawPiece
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 9; j++) {
                if (GameEngine.position[i][j] != 0) {
                    if (GameEngine.isRed(i, j)) {
                        canvas.drawBitmap(resource.redBmp[GameEngine.getType(i, j)], j * resource.splite, i * resource.splite, paint);
                    } else {
                        canvas.drawBitmap(resource.blackBmp[GameEngine.getType(i, j)], j * resource.splite, i * resource.splite, paint);
                    }
                }
            }
        }
        //drawSelect
        canvas.drawBitmap(resource.selectBmp,
                GameEngine.selectedY * resource.splite,
                GameEngine.selectedX * resource.splite,
                paint);
        //drawCursor
        if (GameEngine.lastStep != null) {
            canvas.drawBitmap(resource.cursorBmp,
                    GameEngine.lastStep.fromY * resource.splite,
                    GameEngine.lastStep.fromX * resource.splite,
                    paint);
            canvas.drawBitmap(resource.cursorBmp,
                    GameEngine.lastStep.toY * resource.splite,
                    GameEngine.lastStep.toX * resource.splite,
                    paint);
        }
    }

    private boolean lastRound = false;
    @Override
    public void invalidate() {
        super.invalidate();
        if(lastRound != GameEngine.round) {
            lastRound = GameEngine.round;
            situationSetterWeakRef.get().setRound(GameEngine.round);
            situationSetterWeakRef.get().setSituation(AIEngine.ValueConfig.redValue, AIEngine.ValueConfig.blackValue);
            if (GameEngine.over != 0) {
                situationSetterWeakRef.get().setOver(GameEngine.over == 1);
            }
        }
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        byte x = (byte) (event.getY() * 1.f / resource.splite);
        byte y = (byte) (event.getX() * 1.f / resource.splite);
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                GameEngine.touch(x, y);
                break;
            case MotionEvent.ACTION_UP:
                performClick();
                break;
        }
        invalidate();
        return true;
    }

    private WeakReference<SituationSetter> situationSetterWeakRef;

    public void setRoundSetter(SituationSetter roundSetter) {
        situationSetterWeakRef = new WeakReference<>(roundSetter);
    }
}
