package ru.sibinco.scag.backend.stat.counters;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 11:02:46
 */
public class CATable extends ConfigParamOwner
{
    private String id = null;
    private boolean system = false;

    private int limitsMin = 0;
    private int limitsMax = 0;

    public CATable() {
    }
    public CATable(String id, boolean system) {
        this.id = id; this.system = system;
    }

    public String getId() {
        return id;
    }
    public void setId(String id) {
        this.id = id;
    }

    public boolean isSystem() {
        return system;
    }
    public String getSystem() {
        return Boolean.toString(system);
    }
    public void setSystem(boolean system) {
        this.system = system;
    }

    public int getLimitsMin() {
        return limitsMin;
    }
    public String getLimitsMinString() {
        return Integer.toString(limitsMin);
    }
    public void setLimitsMin(int limitsMin) {
        this.limitsMin = limitsMin;
    }

    public int getLimitsMax() {
        return limitsMax;
    }
    public String getLimitsMaxString() {
        return Integer.toString(limitsMax);
    }
    public void setLimitsMax(int limitsMax) {
        this.limitsMax = limitsMax;
    }
}
