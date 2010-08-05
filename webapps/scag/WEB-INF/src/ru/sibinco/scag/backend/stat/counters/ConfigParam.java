package ru.sibinco.scag.backend.stat.counters;

import java.util.Collection;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 15.02.2010
 * Time: 12:06:46
 */
public class ConfigParam implements java.io.Serializable, Cloneable
{
    public static final String TYPE_STRING = "string";
    public static final String TYPE_INTEGER = "int";
    public static final String TYPE_BOOLEAN = "bool";

    private String name;
    private String type;
    private String value;

    public ConfigParam(){
        
    }

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

    public String toString() {
        return "ConfigParam{" +
                "type=\"" + getType() + "\"" +
                ", name=\"" + getName() + "\"" +
                ", value=\"" + value +
                "\"}";
    }   

    public static String getParameterType(String value){
        //logger.debug("Parameter type: " + value);
        String type;
        if (value.compareTo("true") == 0 || value.compareTo("false") == 0){
            type = TYPE_BOOLEAN;
        } else{
            try{
                Integer.parseInt(value);
                type = TYPE_INTEGER;
            } catch (NumberFormatException e){
                type = TYPE_STRING;
            }
        }
        //logger.debug("Type: "+type);
        return type;
    }

    public static ConfigParam[] getConfigParams(Collection<ConfigParam> configParams){
        return (ConfigParam[]) configParams.toArray(new ConfigParam[configParams.size()]);
    }
}
