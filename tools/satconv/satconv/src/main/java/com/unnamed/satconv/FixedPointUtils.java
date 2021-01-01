package com.unnamed.satconv;

public class FixedPointUtils {
    public static int toFixed(float value) {
        return ((int)(value * 65536.0) & 0xFFFFFFFF);
    }

    public static int toFixed(double value) {
        return toFixed((float)value);
    }
}
