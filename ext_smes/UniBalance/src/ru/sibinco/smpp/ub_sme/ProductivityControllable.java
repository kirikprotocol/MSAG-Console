package ru.sibinco.smpp.ub_sme;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public interface ProductivityControllable {
    public long getEventsCount();

    public void setEventsCounterEnabled(boolean enabled);

    public void resetEventsCounter();

    public long getCounterStartTime();

    public String getName();
}
