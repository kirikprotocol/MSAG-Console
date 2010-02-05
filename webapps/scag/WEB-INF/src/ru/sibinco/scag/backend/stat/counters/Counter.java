package ru.sibinco.scag.backend.stat.counters;

import java.util.Map;
import java.util.Set;
import java.util.HashMap;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 10:38:26
 */
public class Counter
{
    private String id = null;
    private CounterType type = CounterType.ACCUMULATOR;
    private String CATableId = null;

    private final HashMap<String, String> params = new HashMap<String, String>();

    public Counter() {
    }
    public Counter(String id, CounterType type) {
        this.id = id; this.type = type;
    }

    public String getId() {
        return id;
    }
    public void setId(String id) {
        this.id = id;
    }

    public CounterType getType() {
        return type;
    }
    public String getTypeString() {
        return type.toString();
    }
    public void setType(CounterType type) {
        this.type = type;
    }
    public void setType(String type) {
        this.type = CounterType.valueOf(type);
    }

    public Set<Map.Entry<String, String>> getParams() {
        return params.entrySet();
    }
    public String getParam(String name) {
        return params.get(name);
    }
    public void setParam(String name, String value) {
        params.put(name, value);
    }

    public void setCATableId(String CATableId) {
        this.CATableId = CATableId;
    }
    public String getCATableId() {
        return CATableId;
    }

    public boolean equals(final Object o) {
      return (this == o) || ((o instanceof Counter) && id.equals(((Counter)o).id));
    }
    public int hashCode() {
      return id.hashCode();
    }

}
