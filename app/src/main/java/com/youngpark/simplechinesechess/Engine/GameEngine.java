package com.youngpark.simplechinesechess.Engine;

import android.util.Log;

import com.youngpark.simplechinesechess.NativeBridge.AIBridge;
import com.youngpark.simplechinesechess.Step;

import java.util.Stack;

public class GameEngine {

    private static final String TAG = "youngpark_671";

    /**
     * 0    nothing
     * 8 - 14   redBmp
     * 16 - 22  blackBmp
     */

    public static class Constant {
        /**
         * 8 16帅
         * 9 17士
         * 10 18象
         * 11 19马
         * 12 20车
         * 13 21炮
         * 14 22兵
         */
        public static final int
                RedMa = 11,
                BlackMa = 19,
                RedXiang = 10,
                BlackXiang = 18,
                RedShi = 9,
                BlackShi = 17,
                RedShuai = 8,
                BlackShuai = 16,
                RedJu = 12,
                BlackJu = 20,
                RedPao = 13,
                BlackPao = 21,
                RedBing = 14,
                BlackBing = 22,
                Empty = 0;
    }


    public static byte[][] position;
    public static byte selectedX = -1, selectedY = -1;
    private static final byte[][] defaultPosition = {
            {20, 19, 18, 17, 16, 17, 18, 19, 20},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 21, 0, 0, 0, 0, 0, 21, 0},
            {22, 0, 22, 0, 22, 0, 22, 0, 22},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {14, 0, 14, 0, 14, 0, 14, 0, 14},
            {0, 13, 0, 0, 0, 0, 0, 13, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {12, 11, 10, 9, 8, 9, 10, 11, 12}
    };

    public static void init() {
        round = true;
        backUpStack = new Stack<>();
        lastStep = null;
        over = 0;
        moveSoundCount = selectSoundCount = 0;
        startSoundCount++;
        position = new byte[10][9];
        for (int i = 0; i < 10; i++) {
            System.arraycopy(defaultPosition[i], 0, position[i], 0, 9);
        }
        AIEngine.init();
        AIEngine.ValueConfig.detectAllSituation();
        if (GameMode == 2) {
            AIEngine.startThinking();
        }
    }

    public static boolean isRed(int line, int pos) {
        return (position[line][pos] & 8) != 0;
    }

    public static int getType(int line, int pos) {
        if (isRed(line, pos)) {
            return position[line][pos] - 7;
        } else {
            return position[line][pos] - 15;
        }
    }

    private static final short[] maX = {
            -1, -1, 1, 1, -2, -2, 2, 2
    };
    private static final short[] maY = {
            -2, 2, -2, 2, -1, 1, -1, 1
    };
    private static final short[] maEyeX = {
            0, 0, 0, 0, -1, -1, 1, 1
    };
    private static final short[] maEyeY = {
            -1, 1, -1, 1, 0, 0, 0, 0
    };

    private static boolean inBoard(int x, int y) {
        return x >= 0 && x < 10 && y >= 0 && y <= 9;
    }

    private static boolean isMaValid(int moveX, int moveY) {
        for (int i = 0; i < maX.length; i++) {
            if (moveX == maX[i] && moveY == maY[i]) {
                //是马步
                int eyeX = maEyeX[i] + selectedX;
                int eyeY = maEyeY[i] + selectedY;
                if (inBoard(eyeX, eyeY) && position[eyeX][eyeY] == 0) {
                    //马眼无棋子
                    if (inBoard(selectedX + moveX, selectedY + moveY)) {
                        //目标地点在棋盘内
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private static final short[] xiangX = {
            -2, -2, 2, 2
    };
    private static final short[] xiangY = {
            -2, 2, -2, 2
    };
    private static final short[] xiangEyeX = {
            -1, -1, 1, 1
    };
    private static final short[] xiangEyeY = {
            -1, 1, -1, 1
    };

    private static boolean isXiangValid(int moveX, int moveY) {
        for (int i = 0; i < xiangX.length; i++) {
            if (moveX == xiangX[i] && moveY == xiangY[i]) {
                //是象步
                int eyeX = xiangEyeX[i] + selectedX;
                int eyeY = xiangEyeY[i] + selectedY;
                if (inBoard(eyeX, eyeY) && position[eyeX][eyeY] == 0) {
                    //象眼无棋子
                    int targetX = selectedX + moveX;
                    int targetY = selectedY + moveY;
                    if (inBoard(targetX, targetY)) {
                        //目标地点在棋盘内
                        if (isRed(selectedX, selectedY) && targetX > 4) {
                            return true;
                        } else if (!isRed(selectedX, selectedY) && targetX < 5) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    private static final short[][] bingX = {
            {-1, 0, 0},
            {1, 0, 0}
    };
    private static final short[] bingY = {
            0, 1, -1
    };

    private static boolean isBingValid(int moveX, int moveY) {
        int color = isRed(selectedX, selectedY) ? 0 : 1;
        if ((color == 0 && selectedX > 4) || (color == 1 && selectedX < 5)) {
            return moveX == bingX[color][0] && moveY == bingY[0];
        } else {
            for (int i = 0; i < bingY.length; i++) {
                if (moveX == bingX[color][i] && moveY == bingY[i]) {
                    //是兵步
                    int targetX = selectedX + moveX;
                    int targetY = selectedY + moveY;
                    if (inBoard(targetX, targetY)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private static final short[] shiX = {
            -1, -1, 1, 1
    };
    private static final short[] shiY = {
            1, -1, 1, -1
    };

    private static boolean isInNine(int color, int x, int y) {
        //r: fromX 0-2 fromY 3-5
        //b: fromX 7-9 fromY 3-5
        Log.d(TAG, "xy" + x + ", " + y);
        if (y >= 3 && y <= 5) {
            if (color == 0) {
                //红
                return x >= 7 && x <= 9;
            } else {
                return x >= 0 && x <= 2;
            }
        }
        return false;
    }

    private static boolean isShiValid(int moveX, int moveY) {
        int color = isRed(selectedX, selectedY) ? 0 : 1;
        for (int i = 0; i < shiX.length; i++) {
            if (shiX[i] == moveX && shiY[i] == moveY) {
                Log.d(TAG, "isShiStep");
                //是士步
                int targetX = selectedX + moveX;
                int targetY = selectedY + moveY;
                if (isInNine(color, targetX, targetY)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static final short[] shuaiX = {
            1, -1, 0, 0
    };
    private static final short[] shuaiY = {
            0, 0, 1, -1
    };

    private static boolean isShuaiValid(int moveX, int moveY) {
        int color = isRed(selectedX, selectedY) ? 0 : 1;
        for (int i = 0; i < shuaiX.length; i++) {
            if (shuaiX[i] == moveX && shuaiY[i] == moveY) {
                int targetX = selectedX + moveX;
                int targetY = selectedY + moveY;
                if (isInNine(color, targetX, targetY)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static boolean isJuValid(int moveX, int moveY) {
        int targetX = selectedX + moveX;
        int targetY = selectedY + moveY;
        if ((moveX == 0 || moveY == 0) && inBoard(targetX, targetY)) {
            int startX, startY, endX, endY;
            if (selectedX < targetX) {
                startX = selectedX;
                endX = targetX;
            } else {
                startX = targetX;
                endX = selectedX;
            }
            if (selectedY < targetY) {
                startY = selectedY;
                endY = targetY;
            } else {
                startY = targetY;
                endY = selectedY;
            }
            boolean valid = true;
            if (moveX == 0) {
                for (int i = startY + 1; i <= endY - 1; i++) {
                    if (position[selectedX][i] != Constant.Empty) {
                        valid = false;
                        break;
                    }
                }
            } else {
                for (int i = startX + 1; i <= endX - 1; i++) {
                    if (position[i][selectedY] != Constant.Empty) {
                        valid = false;
                        break;
                    }
                }
            }
            return valid;
        }
        return false;
    }

    private static boolean isPaoValid(int moveX, int moveY) {
        int targetX = selectedX + moveX;
        int targetY = selectedY + moveY;
        if ((moveX == 0 || moveY == 0) && inBoard(targetX, targetY)) {
            int startX, startY, endX, endY;
            if (selectedX < targetX) {
                startX = selectedX;
                endX = targetX;
            } else {
                startX = targetX;
                endX = selectedX;
            }
            if (selectedY < targetY) {
                startY = selectedY;
                endY = targetY;
            } else {
                startY = targetY;
                endY = selectedY;
            }
            int count = 0;
            if (moveX == 0) {

                for (int i = startY + 1; i <= endY - 1; i++) {
                    if (position[selectedX][i] != Constant.Empty) {
                        count++;
                    }
                }
            } else {
                for (int i = startX + 1; i <= endX - 1; i++) {
                    if (position[i][selectedY] != Constant.Empty) {
                        count++;
                    }
                }
            }
            if (position[targetX][targetY] == Constant.Empty) {
                return count == 0;
            } else {
                return count == 1;
            }
        }
        return false;
    }

    public static byte getPieceType(int i, int j) {
        if (position[i][j] == Constant.Empty) {
            return 0;
        } else {
            if (isRed(i, j)) {
                return (byte) (position[i][j] - 7);
            } else {
                return (byte) (position[i][j] - 15);
            }
        }
    }

    public static byte getPieceColor(int i, int j) {
        if (position[i][j] == Constant.Empty) {
            return 0;
        } else {
            if (isRed(i, j)) {
                return 1;
            } else {
                return -1;
            }
        }
    }

    public static boolean round = true;//t-r f-b

    public static void touch(byte x, byte y) {
        if (over != 0) return;
        Log.d(TAG, "touch:" + x + ", " + y);
        if (!inBoard(x, y)) return;
        if (selectedX == -1) {
            if (position[x][y] != Constant.Empty) {
                if ((round && isRed(x, y)) || (!round && !isRed(x, y))) {
                    Log.d(TAG, "touch");
                    selectedX = x;
                    selectedY = y;
                    selectSoundCount++;
                }
            }
        } else if (!(x == selectedX && y == selectedY)
                && (position[x][y] == Constant.Empty
                || (position[x][y] != Constant.Empty && isRed(x, y) != isRed(selectedX, selectedY)))) {
            Log.d(TAG, "move");
            int moveX = x - selectedX;
            int moveY = y - selectedY;

            boolean validMove = false;
            switch (position[selectedX][selectedY]) {
                case Constant.RedMa:
                case Constant.BlackMa:
                    validMove = isMaValid(moveX, moveY);
                    break;
                case Constant.RedXiang:
                case Constant.BlackXiang:
                    validMove = isXiangValid(moveX, moveY);
                    break;
                case Constant.BlackBing:
                case Constant.RedBing:
                    validMove = isBingValid(moveX, moveY);
                    break;
                case Constant.BlackJu:
                case Constant.RedJu:
                    validMove = isJuValid(moveX, moveY);
                    break;
                case Constant.BlackPao:
                case Constant.RedPao:
                    validMove = isPaoValid(moveX, moveY);
                    break;
                case Constant.BlackShi:
                case Constant.RedShi:
                    validMove = isShiValid(moveX, moveY);
                    break;
                case Constant.BlackShuai:
                case Constant.RedShuai:
                    validMove = isShuaiValid(moveX, moveY);
                    break;
                default:
                    Log.d(TAG, "???????? WTF????");
            }
            if (validMove) {
                move(x, y);
            }
            selectedX = selectedY = -1;
        } else {
            selectedX = selectedY = -1;
        }
    }

    private static Stack<Step> backUpStack;
    public static Step lastStep = null;

    static int selectSoundCount, moveSoundCount, startSoundCount, winSoundCount;
    public static int over = 0;//0-running 1-win  2-fail 3-exit

    public static int GameMode = 0; // 0:h vs h, 1:h vs ai, 2:ai vs ai

    private static void move(int x, int y) {
        lastStep = new Step();
        lastStep.fromX = selectedX;
        lastStep.fromY = selectedY;
        lastStep.piece = position[selectedX][selectedY];
        lastStep.toX = x;
        lastStep.toY = y;
        lastStep.eat = position[x][y];
        if (lastStep.eat == Constant.BlackShuai) {
            over = 1;
        } else if (lastStep.eat == Constant.RedShuai) {
            over = 2;
        }
        backUpStack.push(lastStep);
        position[x][y] = position[selectedX][selectedY];
        position[selectedX][selectedY] = Constant.Empty;
        round = !round;
        moveSoundCount++;

        AIEngine.ValueConfig.detectMoveSituation();

        switch (GameMode) {
            case 0:
                //do nothing
                break;
            case 1:
                if (!round) {
                    AIEngine.startThinkingAfterHuman();
                }
                break;
            case 2:
                AIEngine.startThinking();
                break;
        }
    }

    public static void backUp() {
        if (!backUpStack.empty()) {
            AIBridge.undoMove();
            selectedY = selectedX = -1;
            Step step = backUpStack.pop();
            position[step.fromX][step.fromY] = step.piece;
            position[step.toX][step.toY] = step.eat;
            round = !round;
            if (backUpStack.empty()) {
                lastStep = null;
            } else {
                lastStep = backUpStack.peek();
            }
            while (backUpStack.size() % 2 != 0) {
                AIBridge.undoMove();
                selectedY = selectedX = -1;
                step = backUpStack.pop();
                position[step.fromX][step.fromY] = step.piece;
                position[step.toX][step.toY] = step.eat;
                round = !round;
                if (backUpStack.empty()) {
                    lastStep = null;
                } else {
                    lastStep = backUpStack.peek();
                }
            }
        }
    }
}
