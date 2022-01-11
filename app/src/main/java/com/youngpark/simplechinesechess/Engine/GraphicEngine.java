package com.youngpark.simplechinesechess.Engine;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.View;

import com.youngpark.simplechinesechess.R;

import java.lang.ref.WeakReference;

public class GraphicEngine {

    public static class Resource {
        private WeakReference<View> viewWeakReference;

        public Bitmap[] redBmp = new Bitmap[8];
        public Bitmap[] blackBmp = new Bitmap[8];
        public Bitmap boardBmp, selectBmp, cursorBmp;
        public int splite;

        public void init(View view) {
            viewWeakReference = new WeakReference<>(view);
            initBitmap();
        }

        private void initBitmap() {
            boardBmp = BitmapFactory.decodeResource(viewWeakReference.get().getResources(), R.drawable.board);
            selectBmp = BitmapFactory.decodeResource(viewWeakReference.get().getResources(), R.drawable.selected);
            cursorBmp = BitmapFactory.decodeResource(viewWeakReference.get().getResources(), R.drawable.cursor);
            Class<?> Rclazz = R.drawable.class;
            try {
                for (int i = 1; i <= 7; i++) {
                    int redResId = (int) Rclazz.getDeclaredField("r" + i).get(null);
                    int blackResId = (int) Rclazz.getDeclaredField("b" + i).get(null);
                    redBmp[i] = BitmapFactory.decodeResource(viewWeakReference.get().getResources(), redResId);
                    blackBmp[i] = BitmapFactory.decodeResource(viewWeakReference.get().getResources(), blackResId);
                }
            } catch (Throwable tr) {
                tr.printStackTrace();
            }
        }

        public void scaleBitmap(int width) {
            int height = (int) ((width * 1.f / boardBmp.getWidth()) * boardBmp.getHeight());
            boardBmp = Bitmap.createScaledBitmap(boardBmp, width, height, true);
            splite = width / 9;

            int pieceSize = (int) ((width / 9.f) - 15);
            selectBmp = Bitmap.createScaledBitmap(selectBmp, pieceSize, pieceSize, true);
            cursorBmp = Bitmap.createScaledBitmap(cursorBmp, pieceSize, pieceSize, true);
            for (int i = 1; i <= 7; i++) {
                redBmp[i] = Bitmap.createScaledBitmap(redBmp[i], pieceSize, pieceSize, true);
                blackBmp[i] = Bitmap.createScaledBitmap(blackBmp[i], pieceSize, pieceSize, true);
            }
        }
    }

    public static class Draw {
        //TODO Not Implement Yet
    }

}
