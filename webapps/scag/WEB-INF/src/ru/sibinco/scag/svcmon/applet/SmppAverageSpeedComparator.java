/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.snap.SmppSnap;

import java.util.Comparator;

public class SmppAverageSpeedComparator implements Comparator {
    int idx;
    boolean asc;

    public SmppAverageSpeedComparator(boolean asc, int idx) {
        this.idx = idx;
        this.asc = asc;
    }

    public int compare(Object o1, Object o2) {
        SmppSnap snap1 = (SmppSnap) o1;
        SmppSnap snap2 = (SmppSnap) o2;
        if (snap1.smppAvgSpeed[idx] == snap2.smppAvgSpeed[idx])
            return 0;
        else if (snap1.smppAvgSpeed[idx] < snap2.smppAvgSpeed[idx])
            return asc ? -1 : 1;
        else
            return asc ? 1 : -1;
    }
}