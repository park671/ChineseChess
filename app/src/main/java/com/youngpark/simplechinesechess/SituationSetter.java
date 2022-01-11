package com.youngpark.simplechinesechess;

public interface SituationSetter {
    void setRound(boolean red);
    void setSituation(int redVal, int blackVal);
    void setOver(boolean win);
}
