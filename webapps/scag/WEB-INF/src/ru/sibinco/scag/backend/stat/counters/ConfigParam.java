package ru.sibinco.scag.backend.stat.counters;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 15.02.2010
 * Time: 12:06:46
 */
public class ConfigParam
{
    private String name;
    private String type;
    private String value;

    public ConfigParam(String name, String type, String value) {
        this.name = name; this.type = type; this.value = value;
    }

    public String getName() {
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public String getType() {
        return type;
    }
    public void setType(String type) {
        this.type = type;
    }
    public String getValue() {
        return value;
    }
    public void setValue(String value) {
        this.value = value;
    }
}
