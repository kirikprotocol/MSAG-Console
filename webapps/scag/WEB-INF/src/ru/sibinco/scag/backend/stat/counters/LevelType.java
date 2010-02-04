package ru.sibinco.scag.backend.stat.counters;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 14:12:30
 */
public enum LevelType
{
    NORMAL  (0x01, "NORMAL"),
    WARNING (0x02, "WARNING"),
    MINOR   (0x03, "MINOR"),
    MAJOR   (0x04, "MAJOR"),
    CRITICAL(0x05, "CRITICAL"),
    UNKNOWN (0xFF, "UNKNOWN"); // TODO: need it ???

    private final byte value;
    private final String name;

    LevelType(int value, String name) {
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
