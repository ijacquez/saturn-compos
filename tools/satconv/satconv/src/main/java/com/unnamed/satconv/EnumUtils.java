package com.unnamed.satconv;

import java.util.EnumSet;

public class EnumUtils {
    public static <E extends Enum<E>> int encode(EnumSet<E> set) {
        int encodedValue = 0;

        for (E enumValue : set) {
            encodedValue |= (1 << enumValue.ordinal());
        }

        return encodedValue;
    }
}
