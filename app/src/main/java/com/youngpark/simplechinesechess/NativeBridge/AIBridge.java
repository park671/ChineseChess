package com.youngpark.simplechinesechess.NativeBridge;

public class AIBridge {
    static {
        System.loadLibrary("ai");
        initNative();
    }
    public static native String test(String msg);
    public static native void initNative();
    public static native void copyMapToNative(byte[] position);
    public static native byte[] copyMapFromNative();
    public static native int getAiResult();
    public static native void humanMove(int start, int end);
    public static native void undoMove();
    public static native void aiResponse();
    public static native void setThinkLimit(int depth, int time);
}
