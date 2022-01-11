#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <android/log.h>
#include <cstring>
#include "com_youngpark_simplechinesechess_NativeBridge_AIBridge.h"


#define TAG "AIEngineResult"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型

extern "C"
JNIEXPORT jstring JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_test(JNIEnv *env, jclass clazz,
                                                                 jstring msg) {
    return env->NewStringUTF("Hello from C++");
}

// 棋子编号
const int PIECE_KING = 0;
const int PIECE_ADVISOR = 1;
const int PIECE_BISHOP = 2;
const int PIECE_KNIGHT = 3;
const int PIECE_ROOK = 4;
const int PIECE_CANNON = 5;
const int PIECE_PAWN = 6;

// 其他常数
int LIMIT_TIME = 1;
const int MAX_GEN_MOVES = 128; // 最大的生成走法数
const int MAX_MOVES = 256;     // 最大的历史走法数
const int LIMIT_DEPTH = 64;    // 最大的搜索深度
const int MATE_VALUE = 10000;  // 最高分值，即将死的分值
const int WIN_VALUE = MATE_VALUE - 100; // 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
const int DRAW_VALUE = 20;     // 和棋时返回的分数(取负值)
const int ADVANCED_VALUE = 3;  // 先行权分值
const int NULL_MARGIN = 400;   // 空步裁剪的子力边界
const int NULL_DEPTH = 2;      // 空步裁剪的裁剪深度
const int HASH_SIZE = 1 << 20; // 置换表大小
const int HASH_ALPHA = 1;      // ALPHA节点的置换表项
const int HASH_BETA = 2;       // BETA节点的置换表项
const int HASH_PV = 3;         // PV节点的置换表项

// 判断棋子是否在棋盘中的数组
static const int ccInBoard[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 判断棋子是否在九宫的数组
static const int ccInFort[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 判断步长是否符合特定走法的数组，1=帅(将)，2=仕(士)，3=相(象)
static const int ccLegalSpan[512] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0
};

// 根据步长判断马是否蹩腿的数组
static const int ccKnightPin[512] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, -16, 0, -16, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 16, 0, 16, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0
};

// 帅(将)的步长
static const int ccKingDelta[4] = {-16, -1, 1, 16};
// 仕(士)的步长
static const int ccAdvisorDelta[4] = {-17, -15, 15, 17};
// 马的步长，以帅(将)的步长作为马腿
static const int ccKnightDelta[4][2] = {{-33, -31},
                                        {-18, 14},
                                        {-14, 18},
                                        {31,  33}};
// 马被将军的步长，以仕(士)的步长作为马腿
static const int ccKnightCheckDelta[4][2] = {{-33, -18},
                                             {-31, -14},
                                             {14,  31},
                                             {18,  33}};

// 棋盘初始设置
static const int cucpcStartup[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 20, 19, 18, 17, 16, 17, 18, 19, 20, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0,
        0, 0, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 14, 0, 14, 0, 14, 0, 14, 0, 14, 0, 0, 0, 0,
        0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 12, 11, 10, 9, 8, 9, 10, 11, 12, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 子力位置价值表
static const int cucvlPiecePos[7][256] = {
        { // 帅(将)
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   1,   1,   1,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   2,   2,   2,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   11,  15,  11,  0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 仕(士)
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   20,  0,   20,  0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   23,  0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   20,  0,   20,  0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 相(象)
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   20,  0,   0,   0,   20,  0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 18,  0,   0,   0,   23,  0,   0,   0,   18,  0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   20,  0,   0,   0,   20,  0,   0,   0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 马
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 90,  90,  90,  96,  90,  96,  90,  90,  90,  0, 0, 0, 0,
                0, 0, 0, 90,  96,  103, 97,  94,  97,  103, 96,  90,  0, 0, 0, 0,
                0, 0, 0, 92,  98,  99,  103, 99,  103, 99,  98,  92,  0, 0, 0, 0,
                0, 0, 0, 93,  108, 100, 107, 100, 107, 100, 108, 93,  0, 0, 0, 0,
                0, 0, 0, 90,  100, 99,  103, 104, 103, 99,  100, 90,  0, 0, 0, 0,
                0, 0, 0, 90,  98,  101, 102, 103, 102, 101, 98,  90,  0, 0, 0, 0,
                0, 0, 0, 92,  94,  98,  95,  98,  95,  98,  94,  92,  0, 0, 0, 0,
                0, 0, 0, 93,  92,  94,  95,  92,  95,  94,  92,  93,  0, 0, 0, 0,
                0, 0, 0, 85,  90,  92,  93,  78,  93,  92,  90,  85,  0, 0, 0, 0,
                0, 0, 0, 88,  85,  90,  88,  90,  88,  90,  85,  88,  0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 车
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 206, 208, 207, 213, 214, 213, 207, 208, 206, 0, 0, 0, 0,
                0, 0, 0, 206, 212, 209, 216, 233, 216, 209, 212, 206, 0, 0, 0, 0,
                0, 0, 0, 206, 208, 207, 214, 216, 214, 207, 208, 206, 0, 0, 0, 0,
                0, 0, 0, 206, 213, 213, 216, 216, 216, 213, 213, 206, 0, 0, 0, 0,
                0, 0, 0, 208, 211, 211, 214, 215, 214, 211, 211, 208, 0, 0, 0, 0,
                0, 0, 0, 208, 212, 212, 214, 215, 214, 212, 212, 208, 0, 0, 0, 0,
                0, 0, 0, 204, 209, 204, 212, 214, 212, 204, 209, 204, 0, 0, 0, 0,
                0, 0, 0, 198, 208, 204, 212, 212, 212, 204, 208, 198, 0, 0, 0, 0,
                0, 0, 0, 200, 208, 206, 212, 200, 212, 206, 208, 200, 0, 0, 0, 0,
                0, 0, 0, 194, 206, 204, 212, 200, 212, 204, 206, 194, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 炮
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 100, 100, 96,  91,  90,  91,  96,  100, 100, 0, 0, 0, 0,
                0, 0, 0, 98,  98,  96,  92,  89,  92,  96,  98,  98,  0, 0, 0, 0,
                0, 0, 0, 97,  97,  96,  91,  92,  91,  96,  97,  97,  0, 0, 0, 0,
                0, 0, 0, 96,  99,  99,  98,  100, 98,  99,  99,  96,  0, 0, 0, 0,
                0, 0, 0, 96,  96,  96,  96,  100, 96,  96,  96,  96,  0, 0, 0, 0,
                0, 0, 0, 95,  96,  99,  96,  100, 96,  99,  96,  95,  0, 0, 0, 0,
                0, 0, 0, 96,  96,  96,  96,  96,  96,  96,  96,  96,  0, 0, 0, 0,
                0, 0, 0, 97,  96,  100, 99,  101, 99,  100, 96,  97,  0, 0, 0, 0,
                0, 0, 0, 96,  97,  98,  98,  98,  98,  98,  97,  96,  0, 0, 0, 0,
                0, 0, 0, 96,  96,  97,  99,  99,  99,  97,  96,  96,  0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // 兵(卒)
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 9,   9,   9,   11,  13,  11,  9,   9,   9,   0, 0, 0, 0,
                0, 0, 0, 19,  24,  34,  42,  44,  42,  34,  24,  19,  0, 0, 0, 0,
                0, 0, 0, 19,  24,  32,  37,  37,  37,  32,  24,  19,  0, 0, 0, 0,
                0, 0, 0, 19,  23,  27,  29,  30,  29,  27,  23,  19,  0, 0, 0, 0,
                0, 0, 0, 14,  18,  20,  27,  29,  27,  20,  18,  14,  0, 0, 0, 0,
                0, 0, 0, 7,   0,   13,  0,   16,  0,   13,  0,   7,   0, 0, 0, 0,
                0, 0, 0, 7,   0,   7,   0,   15,  0,   7,   0,   7,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
};

// 判断棋子是否在棋盘中
inline bool IN_BOARD(int sq) {
    return ccInBoard[sq] != 0;
}

// 判断棋子是否在九宫中
inline bool IN_FORT(int sq) {
    return ccInFort[sq] != 0;
}

// 获得格子的横坐标
inline int RANK_Y(int sq) {
    return sq >> 4;
}

// 获得格子的纵坐标
inline int FILE_X(int sq) {
    return sq & 15;
}

// 根据纵坐标和横坐标获得格子
inline int COORD_XY(int x, int y) {
    return x + (y << 4);
}

// 翻转格子
inline int SQUARE_FLIP(int sq) {
    return 254 - sq;
}

// 纵坐标水平镜像
inline int FILE_FLIP(int x) {
    return 14 - x;
}

// 横坐标垂直镜像
inline int RANK_FLIP(int y) {
    return 15 - y;
}

// 格子水平镜像
inline int MIRROR_SQUARE(int sq) {
    return COORD_XY(FILE_FLIP(FILE_X(sq)), RANK_Y(sq));
}

// 格子水平镜像
inline int SQUARE_FORWARD(int sq, int sd) {
    return sq - 16 + (sd << 5);
}

// 走法是否符合帅(将)的步长
inline bool KING_SPAN(int sqSrc, int sqDst) {
    return ccLegalSpan[sqDst - sqSrc + 256] == 1;
}

// 走法是否符合仕(士)的步长
inline bool ADVISOR_SPAN(int sqSrc, int sqDst) {
    return ccLegalSpan[sqDst - sqSrc + 256] == 2;
}

// 走法是否符合相(象)的步长
inline bool BISHOP_SPAN(int sqSrc, int sqDst) {
    return ccLegalSpan[sqDst - sqSrc + 256] == 3;
}

// 相(象)眼的位置
inline int BISHOP_PIN(int sqSrc, int sqDst) {
    return (sqSrc + sqDst) >> 1;
}

// 马腿的位置
inline int KNIGHT_PIN(int sqSrc, int sqDst) {
    return sqSrc + ccKnightPin[sqDst - sqSrc + 256];
}

// 是否未过河
inline bool HOME_HALF(int sq, int sd) {
    return (sq & 0x80) != (sd << 7);
}

// 是否已过河
inline bool AWAY_HALF(int sq, int sd) {
    return (sq & 0x80) == (sd << 7);
}

// 是否在河的同一边
inline bool SAME_HALF(int sqSrc, int sqDst) {
    return ((sqSrc ^ sqDst) & 0x80) == 0;
}

// 是否在同一行
inline bool SAME_RANK(int sqSrc, int sqDst) {
    return ((sqSrc ^ sqDst) & 0xf0) == 0;
}

// 是否在同一列
inline bool SAME_FILE(int sqSrc, int sqDst) {
    return ((sqSrc ^ sqDst) & 0x0f) == 0;
}

// 获得红黑标记(红子是8，黑子是16)
inline int SIDE_TAG(int sd) {
    return 8 + (sd << 3);
}

// 获得对方红黑标记
inline int OPP_SIDE_TAG(int sd) {
    return 16 - (sd << 3);
}

// 获得走法的起点
inline int SRC(int mv) {
    return mv & 255;
}

// 获得走法的终点
inline int DST(int mv) {
    return mv >> 8;
}

// 根据起点和终点获得走法
inline int MOVE(int sqSrc, int sqDst) {
    return sqSrc + sqDst * 256;
}

// 走法水平镜像
inline int MIRROR_MOVE(int mv) {
    return MOVE(MIRROR_SQUARE(SRC(mv)), MIRROR_SQUARE(DST(mv)));
}

// RC4密码流生成器
struct RC4Struct {
    int s[256];
    int x, y;

    void InitZero();   // 用空密钥初始化密码流生成器
    int Nextint() {  // 生成密码流的下一个字节
        int uc;
        x = (x + 1) & 255;
        y = (y + s[x]) & 255;
        uc = s[x];
        s[x] = s[y];
        s[y] = uc;
        return s[(s[x] + s[y]) & 255];
    }

    unsigned long NextLong() { // 生成密码流的下四个字节
        int uc0, uc1, uc2, uc3;
        uc0 = Nextint();
        uc1 = Nextint();
        uc2 = Nextint();
        uc3 = Nextint();
        return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
    }
};

// 用空密钥初始化密码流生成器
void RC4Struct::InitZero() {
    int i, j;
    int uc;

    x = y = j = 0;
    for (i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i]) & 255;
        uc = s[i];
        s[i] = s[j];
        s[j] = uc;
    }
}

// Zobrist结构
struct ZobristStruct {
    unsigned long dwKey, dwLock0, dwLock1;

    void InitZero() {                 // 用零填充Zobrist
        dwKey = dwLock0 = dwLock1 = 0;
    }

    void InitRC4(RC4Struct &rc4) {        // 用密码流填充Zobrist
        dwKey = rc4.NextLong();
        dwLock0 = rc4.NextLong();
        dwLock1 = rc4.NextLong();
    }

    void Xor(const ZobristStruct &zobr) { // 执行XOR操作
        dwKey ^= zobr.dwKey;
        dwLock0 ^= zobr.dwLock0;
        dwLock1 ^= zobr.dwLock1;
    }

    void Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2) {
        dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
        dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
        dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
    }
};

// Zobrist表
static struct {
    ZobristStruct Player;
    ZobristStruct Table[14][256];
} Zobrist;

// 初始化Zobrist表
static void InitZobrist() {
    int i, j;
    RC4Struct rc4;
    rc4.InitZero();
    Zobrist.Player.InitRC4(rc4);
    for (i = 0; i < 14; i++) {
        for (j = 0; j < 256; j++) {
            Zobrist.Table[i][j].InitRC4(rc4);
        }
    }
}

// 历史走法信息(占4字节)
struct MoveStruct {
    unsigned int wmv;
    int ucpcCaptured, ucbCheck;
    unsigned long dwKey;

    void Set(int mv, int pcCaptured, bool bCheck, unsigned long dwKey_) {
        wmv = mv;
        ucpcCaptured = pcCaptured;
        ucbCheck = bCheck;
        dwKey = dwKey_;
    }
}; // mvs

// 局面结构
struct PositionStruct {
    int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
    int ucpcSquares[256];          // 棋盘上的棋子
    int vlWhite, vlBlack;           // 红、黑双方的子力价值
    int nDistance, nMoveNum;        // 距离根节点的步数，历史走法数
    MoveStruct mvsList[MAX_MOVES];  // 历史走法信息列表
    ZobristStruct zobr;             // Zobrist

    void SetIrrev() {           // 清空(初始化)历史走法信息
        mvsList[0].Set(0, 0, Checked(), zobr.dwKey);
        nMoveNum = 1;
    }

    void Startup();             // 初始化棋盘
    void ChangeSide() {         // 交换走子方
        sdPlayer = 1 - sdPlayer;
        zobr.Xor(Zobrist.Player);
    }

    void AddPiece(int sq, int pc) { // 在棋盘上放一枚棋子
        ucpcSquares[sq] = pc;
        // 红方加分，黑方(注意"cucvlPiecePos"取值要颠倒)减分
        if (pc < 16) {
            vlWhite += cucvlPiecePos[pc - 8][sq];
            zobr.Xor(Zobrist.Table[pc - 8][sq]);
        } else {
            vlBlack += cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
            zobr.Xor(Zobrist.Table[pc - 9][sq]);
        }
    }

    void DelPiece(int sq, int pc) { // 从棋盘上拿走一枚棋子
        ucpcSquares[sq] = 0;
        // 红方减分，黑方(注意"cucvlPiecePos"取值要颠倒)加分
        if (pc < 16) {
            vlWhite -= cucvlPiecePos[pc - 8][sq];
            zobr.Xor(Zobrist.Table[pc - 8][sq]);
        } else {
            vlBlack -= cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
            zobr.Xor(Zobrist.Table[pc - 9][sq]);
        }
    }

    int Evaluate() const {      // 局面评价函数
        return (sdPlayer == 0 ? vlWhite - vlBlack : vlBlack - vlWhite) + ADVANCED_VALUE;
    }

    bool InCheck() const {      // 是否被将军
        return mvsList[nMoveNum - 1].ucbCheck;
    }

    bool Captured() const {     // 上一步是否吃子
        return mvsList[nMoveNum - 1].ucpcCaptured != 0;
    }

    int MovePiece(int mv);                      // 搬一步棋的棋子
    void UndoMovePiece(int mv, int pcCaptured); // 撤消搬一步棋的棋子
    bool MakeMove(int mv);                      // 走一步棋
    void UndoMakeMove() {                   // 撤消走一步棋
        nDistance--;
        nMoveNum--;
        ChangeSide();
        UndoMovePiece(mvsList[nMoveNum].wmv, mvsList[nMoveNum].ucpcCaptured);
    }

    void NullMove() {                       // 走一步空步
        unsigned long dwKey;
        dwKey = zobr.dwKey;
        ChangeSide();
        mvsList[nMoveNum].Set(0, 0, false, dwKey);
        nMoveNum++;
        nDistance++;
    }

    void UndoNullMove() {                   // 撤消走一步空步
        nDistance--;
        nMoveNum--;
        ChangeSide();
    }

    // 生成所有走法，如果"bCapture"为"true"则只生成吃子走法
    int GenerateMoves(int *mvs, bool bCapture = false) const;

    bool LegalMove(int mv) const;               // 判断走法是否合理
    bool Checked() const;                   // 判断是否被将军
    bool IsMate();                          // 判断是否被杀
    int DrawValue() const {                 // 和棋分值
        return (nDistance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
    }

    int RepStatus(int nRecur = 1) const;        // 检测重复局面
    int RepValue(int nRepStatus) const {        // 重复局面分值
        int vlReturn;
        vlReturn = ((nRepStatus & 2) == 0 ? 0 : nDistance - MATE_VALUE) +
                   ((nRepStatus & 4) == 0 ? 0 : MATE_VALUE - nDistance);
        return vlReturn == 0 ? DrawValue() : vlReturn;
    }

    bool NullOkay() const {                 // 判断是否允许空步裁剪
        return (sdPlayer == 0 ? vlWhite : vlBlack) > NULL_MARGIN;
    }
};

// 初始化棋盘
void PositionStruct::Startup() {
    int sq, pc;
    sdPlayer = 0;
    vlWhite = vlBlack = nDistance = 0;
    memset(ucpcSquares, 0, 256);
    zobr.InitZero();
    for (sq = 0; sq < 256; sq++) {
        pc = cucpcStartup[sq];
        ucpcSquares[sq] = pc;
        if (pc != 0) {
            AddPiece(sq, pc);
        }
    }
    SetIrrev();
}

// 搬一步棋的棋子
int PositionStruct::MovePiece(int mv) {
    int sqSrc, sqDst, pc, pcCaptured;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pcCaptured = ucpcSquares[sqDst];
    if (pcCaptured != 0) {
        DelPiece(sqDst, pcCaptured);
    }
    pc = ucpcSquares[sqSrc];
    DelPiece(sqSrc, pc);
    AddPiece(sqDst, pc);
    return pcCaptured;
}

// 撤消搬一步棋的棋子
void PositionStruct::UndoMovePiece(int mv, int pcCaptured) {
    int sqSrc, sqDst, pc;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pc = ucpcSquares[sqDst];
    DelPiece(sqDst, pc);
    AddPiece(sqSrc, pc);
    if (pcCaptured != 0) {
        AddPiece(sqDst, pcCaptured);
    }
}

// 走一步棋
bool PositionStruct::MakeMove(int mv) {
    int pcCaptured;
    unsigned long dwKey;

    dwKey = zobr.dwKey;
    pcCaptured = MovePiece(mv);
    if (Checked()) {
        UndoMovePiece(mv, pcCaptured);
        return false;
    }
    ChangeSide();
    mvsList[nMoveNum].Set(mv, pcCaptured, Checked(), dwKey);
    nMoveNum++;
    nDistance++;
    return true;
}

// "GenerateMoves"参数
const bool GEN_CAPTURE = true;

// 生成所有走法，如果"bCapture"为"true"则只生成吃子走法
int PositionStruct::GenerateMoves(int *mvs, bool bCapture) const {
    int i, j, nGenMoves, nDelta, sqSrc, sqDst;
    int pcSelfSide, pcOppSide, pcSrc, pcDst;
    // 生成所有走法，需要经过以下几个步骤：

    nGenMoves = 0;
    pcSelfSide = SIDE_TAG(sdPlayer);
    pcOppSide = OPP_SIDE_TAG(sdPlayer);
    for (sqSrc = 0; sqSrc < 256; sqSrc++) {

        // 1. 找到一个本方棋子，再做以下判断：
        pcSrc = ucpcSquares[sqSrc];
        if ((pcSrc & pcSelfSide) == 0) {
            continue;
        }

        // 2. 根据棋子确定走法
        switch (pcSrc - pcSelfSide) {
            case PIECE_KING:
                for (i = 0; i < 4; i++) {
                    sqDst = sqSrc + ccKingDelta[i];
                    if (!IN_FORT(sqDst)) {
                        continue;
                    }
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves++;
                    }
                }
                break;
            case PIECE_ADVISOR:
                for (i = 0; i < 4; i++) {
                    sqDst = sqSrc + ccAdvisorDelta[i];
                    if (!IN_FORT(sqDst)) {
                        continue;
                    }
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves++;
                    }
                }
                break;
            case PIECE_BISHOP:
                for (i = 0; i < 4; i++) {
                    sqDst = sqSrc + ccAdvisorDelta[i];
                    if (!(IN_BOARD(sqDst) && HOME_HALF(sqDst, sdPlayer) &&
                          ucpcSquares[sqDst] == 0)) {
                        continue;
                    }
                    sqDst += ccAdvisorDelta[i];
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves++;
                    }
                }
                break;
            case PIECE_KNIGHT:
                for (i = 0; i < 4; i++) {
                    sqDst = sqSrc + ccKingDelta[i];
                    if (ucpcSquares[sqDst] != 0) {
                        continue;
                    }
                    for (j = 0; j < 2; j++) {
                        sqDst = sqSrc + ccKnightDelta[i][j];
                        if (!IN_BOARD(sqDst)) {
                            continue;
                        }
                        pcDst = ucpcSquares[sqDst];
                        if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                            mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                            nGenMoves++;
                        }
                    }
                }
                break;
            case PIECE_ROOK:
                for (i = 0; i < 4; i++) {
                    nDelta = ccKingDelta[i];
                    sqDst = sqSrc + nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst == 0) {
                            if (!bCapture) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves++;
                            }
                        } else {
                            if ((pcDst & pcOppSide) != 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves++;
                            }
                            break;
                        }
                        sqDst += nDelta;
                    }
                }
                break;
            case PIECE_CANNON:
                for (i = 0; i < 4; i++) {
                    nDelta = ccKingDelta[i];
                    sqDst = sqSrc + nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst == 0) {
                            if (!bCapture) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves++;
                            }
                        } else {
                            break;
                        }
                        sqDst += nDelta;
                    }
                    sqDst += nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst != 0) {
                            if ((pcDst & pcOppSide) != 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves++;
                            }
                            break;
                        }
                        sqDst += nDelta;
                    }
                }
                break;
            case PIECE_PAWN:
                sqDst = SQUARE_FORWARD(sqSrc, sdPlayer);
                if (IN_BOARD(sqDst)) {
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves++;
                    }
                }
                if (AWAY_HALF(sqSrc, sdPlayer)) {
                    for (nDelta = -1; nDelta <= 1; nDelta += 2) {
                        sqDst = sqSrc + nDelta;
                        if (IN_BOARD(sqDst)) {
                            pcDst = ucpcSquares[sqDst];
                            if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves++;
                            }
                        }
                    }
                }
                break;
        }
    }
    return nGenMoves;
}

// 判断走法是否合理
bool PositionStruct::LegalMove(int mv) const {
    int sqSrc, sqDst, sqPin;
    int pcSelfSide, pcSrc, pcDst, nDelta;
    // 判断走法是否合法，需要经过以下的判断过程：

    // 1. 判断起始格是否有自己的棋子
    sqSrc = SRC(mv);
    pcSrc = ucpcSquares[sqSrc];
    pcSelfSide = SIDE_TAG(sdPlayer);
    if ((pcSrc & pcSelfSide) == 0) {
        return false;
    }

    // 2. 判断目标格是否有自己的棋子
    sqDst = DST(mv);
    pcDst = ucpcSquares[sqDst];
    if ((pcDst & pcSelfSide) != 0) {
        return false;
    }

    // 3. 根据棋子的类型检查走法是否合理
    switch (pcSrc - pcSelfSide) {
        case PIECE_KING:
            return IN_FORT(sqDst) && KING_SPAN(sqSrc, sqDst);
        case PIECE_ADVISOR:
            return IN_FORT(sqDst) && ADVISOR_SPAN(sqSrc, sqDst);
        case PIECE_BISHOP:
            return SAME_HALF(sqSrc, sqDst) && BISHOP_SPAN(sqSrc, sqDst) &&
                   ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0;
        case PIECE_KNIGHT:
            sqPin = KNIGHT_PIN(sqSrc, sqDst);
            return sqPin != sqSrc && ucpcSquares[sqPin] == 0;
        case PIECE_ROOK:
        case PIECE_CANNON:
            if (SAME_RANK(sqSrc, sqDst)) {
                nDelta = (sqDst < sqSrc ? -1 : 1);
            } else if (SAME_FILE(sqSrc, sqDst)) {
                nDelta = (sqDst < sqSrc ? -16 : 16);
            } else {
                return false;
            }
            sqPin = sqSrc + nDelta;
            while (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                sqPin += nDelta;
            }
            if (sqPin == sqDst) {
                return pcDst == 0 || pcSrc - pcSelfSide == PIECE_ROOK;
            } else if (pcDst != 0 && pcSrc - pcSelfSide == PIECE_CANNON) {
                sqPin += nDelta;
                while (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                    sqPin += nDelta;
                }
                return sqPin == sqDst;
            } else {
                return false;
            }
        case PIECE_PAWN:
            if (AWAY_HALF(sqDst, sdPlayer) && (sqDst == sqSrc - 1 || sqDst == sqSrc + 1)) {
                return true;
            }
            return sqDst == SQUARE_FORWARD(sqSrc, sdPlayer);
        default:
            return false;
    }
}

// 判断是否被将军
bool PositionStruct::Checked() const {
    int i, j, sqSrc, sqDst;
    int pcSelfSide, pcOppSide, pcDst, nDelta;
    pcSelfSide = SIDE_TAG(sdPlayer);
    pcOppSide = OPP_SIDE_TAG(sdPlayer);
    // 找到棋盘上的帅(将)，再做以下判断：

    for (sqSrc = 0; sqSrc < 256; sqSrc++) {
        if (ucpcSquares[sqSrc] != pcSelfSide + PIECE_KING) {
            continue;
        }

        // 1. 判断是否被对方的兵(卒)将军
        if (ucpcSquares[SQUARE_FORWARD(sqSrc, sdPlayer)] == pcOppSide + PIECE_PAWN) {
            return true;
        }
        for (nDelta = -1; nDelta <= 1; nDelta += 2) {
            if (ucpcSquares[sqSrc + nDelta] == pcOppSide + PIECE_PAWN) {
                return true;
            }
        }

        // 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
        for (i = 0; i < 4; i++) {
            if (ucpcSquares[sqSrc + ccAdvisorDelta[i]] != 0) {
                continue;
            }
            for (j = 0; j < 2; j++) {
                pcDst = ucpcSquares[sqSrc + ccKnightCheckDelta[i][j]];
                if (pcDst == pcOppSide + PIECE_KNIGHT) {
                    return true;
                }
            }
        }

        // 3. 判断是否被对方的车或炮将军(包括将帅对脸)
        for (i = 0; i < 4; i++) {
            nDelta = ccKingDelta[i];
            sqDst = sqSrc + nDelta;
            while (IN_BOARD(sqDst)) {
                pcDst = ucpcSquares[sqDst];
                if (pcDst != 0) {
                    if (pcDst == pcOppSide + PIECE_ROOK || pcDst == pcOppSide + PIECE_KING) {
                        return true;
                    }
                    break;
                }
                sqDst += nDelta;
            }
            sqDst += nDelta;
            while (IN_BOARD(sqDst)) {
                int pcDst = ucpcSquares[sqDst];
                if (pcDst != 0) {
                    if (pcDst == pcOppSide + PIECE_CANNON) {
                        return true;
                    }
                    break;
                }
                sqDst += nDelta;
            }
        }
        return false;
    }
    return false;
}

// 判断是否被杀
bool PositionStruct::IsMate() {
    int i, nGenMoveNum, pcCaptured;
    int mvs[MAX_GEN_MOVES];

    nGenMoveNum = GenerateMoves(mvs);
    for (i = 0; i < nGenMoveNum; i++) {
        pcCaptured = MovePiece(mvs[i]);
        if (!Checked()) {
            UndoMovePiece(mvs[i], pcCaptured);
            return false;
        } else {
            UndoMovePiece(mvs[i], pcCaptured);
        }
    }
    return true;
}

// 检测重复局面
int PositionStruct::RepStatus(int nRecur) const {
    bool bSelfSide, bPerpCheck, bOppPerpCheck;
    const MoveStruct *lpmvs;

    bSelfSide = false;
    bPerpCheck = bOppPerpCheck = true;
    lpmvs = mvsList + nMoveNum - 1;
    while (lpmvs->wmv != 0 && lpmvs->ucpcCaptured == 0) {
        if (bSelfSide) {
            bPerpCheck = bPerpCheck && lpmvs->ucbCheck;
            if (lpmvs->dwKey == zobr.dwKey) {
                nRecur--;
                if (nRecur == 0) {
                    return 1 + (bPerpCheck ? 2 : 0) + (bOppPerpCheck ? 4 : 0);
                }
            }
        } else {
            bOppPerpCheck = bOppPerpCheck && lpmvs->ucbCheck;
        }
        bSelfSide = !bSelfSide;
        lpmvs--;
    }
    return 0;
}

static PositionStruct pos; // 局面实例

// 置换表项结构
struct HashItem {
    int ucDepth, ucFlag;
    int svl;
    unsigned int wmv, wReserved;
    unsigned long dwLock0, dwLock1;
};

// 与搜索有关的全局变量
static struct {
    int mvResult;                  // 电脑走的棋
    int nHistoryTable[65536];      // 历史表
    int mvKillers[LIMIT_DEPTH][2]; // 杀手走法表
    HashItem HashTable[HASH_SIZE]; // 置换表
} Search;

// 提取置换表项
static int ProbeHash(int vlAlpha, int vlBeta, int nDepth, int &mv) {
    bool bMate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件
    HashItem hsh;

    hsh = Search.HashTable[pos.zobr.dwKey & (HASH_SIZE - 1)];
    if (hsh.dwLock0 != pos.zobr.dwLock0 || hsh.dwLock1 != pos.zobr.dwLock1) {
        mv = 0;
        return -MATE_VALUE;
    }
    mv = hsh.wmv;
    bMate = false;
    if (hsh.svl > WIN_VALUE) {
        hsh.svl -= pos.nDistance;
        bMate = true;
    } else if (hsh.svl < -WIN_VALUE) {
        hsh.svl += pos.nDistance;
        bMate = true;
    }
    if (hsh.ucDepth >= nDepth || bMate) {
        if (hsh.ucFlag == HASH_BETA) {
            return (hsh.svl >= vlBeta ? hsh.svl : -MATE_VALUE);
        } else if (hsh.ucFlag == HASH_ALPHA) {
            return (hsh.svl <= vlAlpha ? hsh.svl : -MATE_VALUE);
        }
        return hsh.svl;
    }
    return -MATE_VALUE;
};

// 保存置换表项
static void RecordHash(int nFlag, int vl, int nDepth, int mv) {
    HashItem hsh;
    hsh = Search.HashTable[pos.zobr.dwKey & (HASH_SIZE - 1)];
    if (hsh.ucDepth > nDepth) {
        return;
    }
    hsh.ucFlag = nFlag;
    hsh.ucDepth = nDepth;
    if (vl > WIN_VALUE) {
        hsh.svl = vl + pos.nDistance;
    } else if (vl < -WIN_VALUE) {
        hsh.svl = vl - pos.nDistance;
    } else {
        hsh.svl = vl;
    }
    hsh.wmv = mv;
    hsh.dwLock0 = pos.zobr.dwLock0;
    hsh.dwLock1 = pos.zobr.dwLock1;
    Search.HashTable[pos.zobr.dwKey & (HASH_SIZE - 1)] = hsh;
};

// MVV/LVA每种子力的价值
static int cucMvvLva[24] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 1, 1, 3, 4, 3, 2, 0,
        5, 1, 1, 3, 4, 3, 2, 0
};

// 求MVV/LVA值
inline int MvvLva(int mv) {
    return (cucMvvLva[pos.ucpcSquares[DST(mv)]] << 3) - cucMvvLva[pos.ucpcSquares[SRC(mv)]];
}

// "qsort"按MVV/LVA值排序的比较函数
static int CompareMvvLva(const void *lpmv1, const void *lpmv2) {
    return MvvLva(*(int *) lpmv2) - MvvLva(*(int *) lpmv1);
}

// "qsort"按历史表排序的比较函数
static int CompareHistory(const void *lpmv1, const void *lpmv2) {
    return Search.nHistoryTable[*(int *) lpmv2] - Search.nHistoryTable[*(int *) lpmv1];
}


// 走法排序阶段
const int PHASE_HASH = 0;
const int PHASE_KILLER_1 = 1;
const int PHASE_KILLER_2 = 2;
const int PHASE_GEN_MOVES = 3;
const int PHASE_REST = 4;

// 走法排序结构
struct SortStruct {
    int mvHash, mvKiller1, mvKiller2; // 置换表走法和两个杀手走法
    int nPhase, nIndex, nGenMoves;    // 当前阶段，当前采用第几个走法，总共有几个走法
    int mvs[MAX_GEN_MOVES];           // 所有的走法

    void Init(int mvHash_) { // 初始化，设定置换表走法和两个杀手走法
        mvHash = mvHash_;
        mvKiller1 = Search.mvKillers[pos.nDistance][0];
        mvKiller2 = Search.mvKillers[pos.nDistance][1];
        nPhase = PHASE_HASH;
    }

    int Next(); // 得到下一个走法
};

// 得到下一个走法
int SortStruct::Next() {
    int mv;
    switch (nPhase) {
        // "nPhase"表示着法启发的若干阶段，依次为：

        // 0. 置换表着法启发，完成后立即进入下一阶段；
        case PHASE_HASH:
            nPhase = PHASE_KILLER_1;
            if (mvHash != 0) {
                return mvHash;
            }
            // 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同

            // 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
        case PHASE_KILLER_1:
            nPhase = PHASE_KILLER_2;
            if (mvKiller1 != mvHash && mvKiller1 != 0 && pos.LegalMove(mvKiller1)) {
                return mvKiller1;
            }

            // 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
        case PHASE_KILLER_2:
            nPhase = PHASE_GEN_MOVES;
            if (mvKiller2 != mvHash && mvKiller2 != 0 && pos.LegalMove(mvKiller2)) {
                return mvKiller2;
            }

            // 3. 生成所有着法，完成后立即进入下一阶段；
        case PHASE_GEN_MOVES:
            nPhase = PHASE_REST;
            nGenMoves = pos.GenerateMoves(mvs);
            qsort(mvs, nGenMoves, sizeof(int), CompareHistory);
            nIndex = 0;

            // 4. 对剩余着法做历史表启发；
        case PHASE_REST:
            while (nIndex < nGenMoves) {
                mv = mvs[nIndex];
                nIndex++;
                if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2) {
                    return mv;
                }
            }

            // 5. 没有着法了，返回零。
        default:
            return 0;
    }
}

// 对最佳走法的处理
inline void SetBestMove(int mv, int nDepth) {
    int *lpmvKillers;
    Search.nHistoryTable[mv] += nDepth * nDepth;
    lpmvKillers = Search.mvKillers[pos.nDistance];
    if (lpmvKillers[0] != mv) {
        lpmvKillers[1] = lpmvKillers[0];
        lpmvKillers[0] = mv;
    }
}

// 静态(Quiescence)搜索过程
static int SearchQuiesc(int vlAlpha, int vlBeta) {
    int i, nGenMoves;
    int vl, vlBest;
    int mvs[MAX_GEN_MOVES];
    // 一个静态搜索分为以下几个阶段

    // 1. 检查重复局面
    vl = pos.RepStatus();
    if (vl != 0) {
        return pos.RepValue(vl);
    }

    // 2. 到达极限深度就返回局面评价
    if (pos.nDistance == LIMIT_DEPTH) {
        return pos.Evaluate();
    }

    // 3. 初始化最佳值
    vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)

    if (pos.InCheck()) {
        // 4. 如果被将军，则生成全部走法
        nGenMoves = pos.GenerateMoves(mvs);
        qsort(mvs, nGenMoves, sizeof(int), CompareHistory);
    } else {

        // 5. 如果不被将军，先做局面评价
        vl = pos.Evaluate();
        if (vl > vlBest) {
            vlBest = vl;
            if (vl >= vlBeta) {
                return vl;
            }
            if (vl > vlAlpha) {
                vlAlpha = vl;
            }
        }

        // 6. 如果局面评价没有截断，再生成吃子走法
        nGenMoves = pos.GenerateMoves(mvs, GEN_CAPTURE);
        qsort(mvs, nGenMoves, sizeof(int), CompareMvvLva);
    }

    // 7. 逐一走这些走法，并进行递归
    for (i = 0; i < nGenMoves; i++) {
        if (pos.MakeMove(mvs[i])) {
            vl = -SearchQuiesc(-vlBeta, -vlAlpha);
            pos.UndoMakeMove();

            // 8. 进行Alpha-Beta大小判断和截断
            if (vl > vlBest) {    // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
                vlBest = vl;        // "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
                if (vl >= vlBeta) { // 找到一个Beta走法
                    return vl;        // Beta截断
                }
                if (vl > vlAlpha) { // 找到一个PV走法
                    vlAlpha = vl;     // 缩小Alpha-Beta边界
                }
            }
        }
    }

    // 9. 所有走法都搜索完了，返回最佳值
    return vlBest == -MATE_VALUE ? pos.nDistance - MATE_VALUE : vlBest;
}

// "SearchFull"的参数
const bool NO_NULL = true;

// 超出边界(Fail-Soft)的Alpha-Beta搜索过程
static int SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false) {
    int nHashFlag, vl, vlBest;
    int mv, mvBest, mvHash;
    SortStruct Sort;
    // 一个Alpha-Beta完全搜索分为以下几个阶段

    if (pos.nDistance > 0) {
        // 1. 到达水平线，则调用静态搜索(注意：由于空步裁剪，深度可能小于零)
        if (nDepth <= 0) {
            return SearchQuiesc(vlAlpha, vlBeta);
        }

        // 1-1. 检查重复局面(注意：不要在根节点检查，否则就没有走法了)
        vl = pos.RepStatus();
        if (vl != 0) {
            return pos.RepValue(vl);
        }

        // 1-2. 到达极限深度就返回局面评价
        if (pos.nDistance == LIMIT_DEPTH) {
            return pos.Evaluate();
        }

        // 1-3. 尝试置换表裁剪，并得到置换表走法
        vl = ProbeHash(vlAlpha, vlBeta, nDepth, mvHash);
        if (vl > -MATE_VALUE) {
            return vl;
        }

        // 1-4. 尝试空步裁剪(根节点的Beta值是"MATE_VALUE"，所以不可能发生空步裁剪)
        if (!bNoNull && !pos.InCheck() && pos.NullOkay()) {
            pos.NullMove();
            vl = -SearchFull(-vlBeta, 1 - vlBeta, nDepth - NULL_DEPTH - 1, NO_NULL);
            pos.UndoNullMove();
            if (vl >= vlBeta) {
                return vl;
            }
        }
    } else {
        mvHash = 0;
    }

    // 2. 初始化最佳值和最佳走法
    nHashFlag = HASH_ALPHA;
    vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)
    mvBest = 0;           // 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

    // 3. 初始化走法排序结构
    Sort.Init(mvHash);

    // 4. 逐一走这些走法，并进行递归
    while ((mv = Sort.Next()) != 0) {
        if (pos.MakeMove(mv)) {
            // 将军延伸
            vl = -SearchFull(-vlBeta, -vlAlpha, pos.InCheck() ? nDepth : nDepth - 1);
            pos.UndoMakeMove();

            // 5. 进行Alpha-Beta大小判断和截断
            if (vl > vlBest) {    // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
                vlBest = vl;        // "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
                if (vl >= vlBeta) { // 找到一个Beta走法
                    nHashFlag = HASH_BETA;
                    mvBest = mv;      // Beta走法要保存到历史表
                    break;            // Beta截断
                }
                if (vl > vlAlpha) { // 找到一个PV走法
                    nHashFlag = HASH_PV;
                    mvBest = mv;      // PV走法要保存到历史表
                    vlAlpha = vl;     // 缩小Alpha-Beta边界
                }
            }
        }
    }

    // 5. 所有走法都搜索完了，把最佳走法(不能是Alpha走法)保存到历史表，返回最佳值
    if (vlBest == -MATE_VALUE) {
        // 如果是杀棋，就根据杀棋步数给出评价
        return pos.nDistance - MATE_VALUE;
    }
    // 记录到置换表
    RecordHash(nHashFlag, vlBest, nDepth, mvBest);
    if (mvBest != 0) {
        // 如果不是Alpha走法，就将最佳走法保存到历史表
        SetBestMove(mvBest, nDepth);
        if (pos.nDistance == 0) {
            // 搜索根节点时，总是有一个最佳走法(因为全窗口搜索不会超出边界)，将这个走法保存下来
            Search.mvResult = mvBest;
        }
    }
    return vlBest;
}

// 迭代加深搜索过程
static void SearchMain() {
    int i, vl;
    long t;
    // 初始化
    memset(Search.nHistoryTable, 0, 65536 * sizeof(int));       // 清空历史表
    memset(Search.mvKillers, 0, LIMIT_DEPTH * 2 * sizeof(int)); // 清空杀手走法表
    memset(Search.HashTable, 0, HASH_SIZE * sizeof(HashItem));  // 清空置换表
    t = clock();       // 初始化定时器
    pos.nDistance = 0; // 初始步数

    // 迭代加深过程
    for (i = 1; i <= LIMIT_DEPTH; i++) {
        vl = SearchFull(-MATE_VALUE, MATE_VALUE, i);
        // 搜索到杀棋，就终止搜索
        if (vl > WIN_VALUE || vl < -WIN_VALUE) {
            break;
        }
        // 超过10秒，就终止搜索
        if (clock() - t > CLOCKS_PER_SEC * LIMIT_TIME) {
            break;
        }
    }
}


static void init() {
    LOGD("Native Init was called !", NULL);
    pos.Startup();
    InitZobrist();
}

static void AIMove() {
    SearchMain();
    pos.MakeMove(Search.mvResult);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_initNative(JNIEnv *env, jclass type) {
    init();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_copyMapToNative(JNIEnv *env,
                                                                            jclass type,
                                                                            jbyteArray position_) {
    jbyte *position = env->GetByteArrayElements(position_, NULL);
    for (int i = 0; i < 256; i++) {
        pos.ucpcSquares[i] = position[i];
    }
    env->ReleaseByteArrayElements(position_, position, 0);
    AIMove();

    jclass clzz = env->FindClass("com/youngpark/simplechinesechess/Engine/AIEngine");
    jmethodID methodID = env->GetStaticMethodID(clzz, "AIThinkOver", "()V");
    env->CallStaticVoidMethod(clzz, methodID);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_copyMapFromNative(JNIEnv *env,
                                                                              jclass type) {

    jbyteArray position_ = env->NewByteArray(256);
    jbyte *position = env->GetByteArrayElements(position_, 0);
    for (int i = 0; i < 256; i++) {
        position[i] = (jbyte)pos.ucpcSquares[i];
    }
    env->ReleaseByteArrayElements(position_, position, 0);
    return position_;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_getAiResult(JNIEnv *env, jclass type) {
    return Search.mvResult;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_humanMove(JNIEnv *env, jclass type,
                                                                      jint start, jint end) {
    int mv = MOVE(start, end);
    int ss = SRC(mv);
    int ee = DST(mv);
    LOGD("human move from %d, %d   %d ,%d", start, ss, end, ee);
    bool leagleMv = pos.LegalMove(mv);
    bool ans = pos.MakeMove(mv);
    LOGD("human move ans = %s, leagle = %s", ans ? "yes" : "no", leagleMv ? "y" : "n");
    AIMove();
    jclass clzz = env->FindClass("com/youngpark/simplechinesechess/Engine/AIEngine");
    jmethodID methodID = env->GetStaticMethodID(clzz, "AIThinkOver", "()V");
    env->CallStaticVoidMethod(clzz, methodID);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_undoMove(JNIEnv *env, jclass type) {
    pos.UndoMakeMove();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_aiResponse(JNIEnv *env, jclass type) {
    AIMove();
    jclass clzz = env->FindClass("com/youngpark/simplechinesechess/Engine/AIEngine");
    jmethodID methodID = env->GetStaticMethodID(clzz, "AIThinkOver", "()V");
    env->CallStaticVoidMethod(clzz, methodID);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngpark_simplechinesechess_NativeBridge_AIBridge_setThinkDepth(JNIEnv *env, jclass type,
                                                                          jint depth,jint time) {
    LIMIT_TIME = time;
}