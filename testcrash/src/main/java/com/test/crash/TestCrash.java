package com.test.crash;

/**
 * FileName: TestCrash
 * Author: gacmy
 * Date: 2021/1/29 8:57 AM
 * Description:
 */
public class TestCrash {

    static {
        System.loadLibrary("testcrash");
    }
    public native void initNative();
}
