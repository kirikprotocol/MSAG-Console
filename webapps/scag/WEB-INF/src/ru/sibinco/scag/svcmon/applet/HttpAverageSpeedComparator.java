/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.snap.HttpSnap;

import java.util.Comparator;

/**
 * The <code>HttpAverageSpeedComparator</code> class represents
 * <p><p/>
 * Date: 26.12.2005
 * Time: 17:48:13
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpAverageSpeedComparator implements Comparator {
    int idx;
    boolean asc;

    public HttpAverageSpeedComparator(boolean asc, int idx) {
        this.idx = idx;
        this.asc = asc;
    }

    public int compare(Object o1, Object o2) {
        HttpSnap snap1 = (HttpSnap) o1;
        HttpSnap snap2 = (HttpSnap) o2;
        if (snap1.httpAvgSpeed[idx] == snap2.httpAvgSpeed[idx])
            return 0;
        else if (snap1.httpAvgSpeed[idx] < snap2.httpAvgSpeed[idx])
            return asc ? -1 : 1;
        else
            return asc ? 1 : -1;
    }
}