package com.chen.cyplayer.enums;

/**
 * @author Created by CHEN on 2019/3/12
 * @email 188669@163.com
 */
public enum MuteEnum {
    MUTE_RIGHT("RIGHT",0),
    MUTE_LEFT("LEFT",1),
    MUTE_CENTER("CENTER", 2);

    private String name;
    private int value;

    MuteEnum(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
