package ru.sibinco.scag.backend.stat.counters;

import java.util.LinkedList;
import java.util.Iterator;
import java.util.List;

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
    
    private final LinkedList<Limit> limits = new LinkedList<Limit>();

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
    public String getLimitsMinStr() {
        return Integer.toString(limitsMin);
    }
    public void setLimitsMin(int limitsMin) {
        this.limitsMin = limitsMin;
    }

    public int getLimitsMax() {
        return limitsMax;
    }
    public String getLimitsMaxStr() {
        return Integer.toString(limitsMax);
    }
    public void setLimitsMax(int limitsMax) {
        this.limitsMax = limitsMax;
    }

    public List<Limit> getLimits() {
        return this.limits;
    }    

    public void addLimit(String percent, String severity) {
        limits.add(new Limit(percent, severity));
    }
}
