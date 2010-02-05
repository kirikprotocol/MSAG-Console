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
    private final String id;
    private final CounterType type;

    private final HashMap<String, String> params = new HashMap<String, String>();
    private String CATableId = null;

    public Counter(String id, CounterType type) {
        this.id = id; this.type = type;
    }

    public String getId() {
        return id;
    }
    public String getType() {
        return type.toString();
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
