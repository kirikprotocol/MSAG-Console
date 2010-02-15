package ru.sibinco.scag.backend.stat.counters;

import java.util.HashMap;
import java.util.Collection;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 15.02.2010
 * Time: 14:42:26
 */
public class ConfigParamOwner 
{
    protected final HashMap<String, ConfigParam> params = new HashMap<String, ConfigParam>();

    public Collection<ConfigParam> getParams() {
        return params.values();
    }
    public ConfigParam getParam(String name) {
        return params.get(name);
    }
    public void setParam(ConfigParam param) {
        params.put(param.getName(), param);
    }
    public void setParam(String name, String type, String value) {
        setParam(new ConfigParam(name, type, value));
    }
}
