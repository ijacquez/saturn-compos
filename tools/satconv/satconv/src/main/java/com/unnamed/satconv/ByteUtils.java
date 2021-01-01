package com.unnamed.satconv;

import java.util.ArrayList;

public class ByteUtils {
    public static byte[] toPrimitiveBytes(Byte[] bytes) {
        byte[] convertedBytes = new byte[bytes.length];

        for (int i = 0; i < bytes.length; i++) {
            convertedBytes[i] = bytes[i];
        }

        return convertedBytes;
    }

    public static byte[] toPrimitiveBytes(ArrayList<Byte> bytes) {
        byte[] convertedBytes = new byte[bytes.size()];

        for (int i = 0; i < convertedBytes.length; i++) {
            convertedBytes[i] = bytes.get(i);
        }

        return convertedBytes;
    }
}
