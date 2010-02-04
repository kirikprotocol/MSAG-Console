package ru.sibinco.scag.backend.stat.counters;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 11:02:46
 */
public class CATable
{
    private final String id;
    private final boolean system;

    private int limitsMin = 0;
    private int limitsMax = 0;

    public CATable(String id, boolean system) {
        this.id = id; this.system = system;
    }

    public String getId() {
        return id;
    }
    public boolean isSystem() {
        return system;
    }

    public String getSystem() {
        return Boolean.toString(system);
    }
    public String getLimitsMin() {
        return Integer.toString(limitsMin);
    }
    public String getLimitsMax() {
        return Integer.toString(limitsMax);
    }

}
