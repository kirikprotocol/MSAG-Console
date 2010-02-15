package ru.sibinco.scag.backend.stat.counters;

import java.util.LinkedList;
import java.util.Iterator;

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

    public class Limit
    {
        private int percent = 0;
        private LevelType level;

        public Limit(int percent, LevelType level) {
            setPercent(percent); setLevel(level);
        }
        public Limit(int percent, String level) {
            setPercent(percent); setLevel(level);
        }
        public Limit(String percent, String level) {
            setPercent(percent); setLevel(level);
        }

        public LevelType getLevel() {
            return level;
        }
        public String getLevelStr() {
            return level.toString();
        }
        public void setLevel(LevelType level) {
            this.level = level;
        }
        public void setLevel(String level) {
            this.level = LevelType.valueOf(level);
        }

        public int getPercent() {
            return percent;
        }
        public String getPercentStr() {
            return Integer.toString(percent);
        }
        public void setPercent(int percent) {
            this.percent = percent;
        }
        public void setPercent(String percent) {
            this.percent = Integer.valueOf(percent);
        }
    }
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

    public Iterator<Limit> getLimits() {
        return limits.iterator();
    }
    public void addLimit(String percent, String severity) {
        limits.add(new CATable.Limit(percent, severity));
    }
}
