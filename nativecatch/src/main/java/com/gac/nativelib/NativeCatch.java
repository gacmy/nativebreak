package com.gac.nativelib;

/**
 * FileName: NativeCatch
 * Author: gacmy
 * Date: 2021/1/28 8:31 PM
 * Description:
 */
public class NativeCatch {

    private static NativeCatchListener mListener;

    static {
        System.loadLibrary("catch");
    }

    public static void setListener(NativeCatchListener listener){
        mListener = listener;
    }

    public static void catchNative(String msg){
       if(mListener != null){
          mListener.catchCrash(msg);
       }
    }

    public static void unRegister(){
        mListener = null;
    }

    public interface NativeCatchListener{
        void catchCrash(String msg);
    }

    public native static void init();
}
