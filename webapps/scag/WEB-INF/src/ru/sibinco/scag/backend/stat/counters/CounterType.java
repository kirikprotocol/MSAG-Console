package ru.sibinco.scag.backend.stat.counters;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 12:11:28
 */
public enum CounterType
{
    ACCUMULATOR (0x01, "ACCUMULATOR"),
    INTEGRAL    (0x02, "INTEGRAL"),
    AVERAGE     (0x03, "AVERAGE"),
    HISTOGRAMM  (0x04, "HISTOGRAMM");

    private final byte value;
    private final String name;

    CounterType(int value, String name) {
        this.value = (byte)value; this.name = name;
    }

    public String getName() {
        return name;
    }
    public byte getValue() {
        return value;
    }

    public String toString() {
        return name;
    }
}
