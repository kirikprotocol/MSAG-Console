/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.applet;

import ru.sibinco.scag.scmon.snap.HttpSnap;

import java.util.Comparator;

/**
 * The <code>HttpSpeedComparator</code> class represents
 * <p><p/>
 * Date: 11.01.2006
 * Time: 15:22:22
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpSpeedComparator   implements Comparator {

    int idx;
    boolean asc;

    public HttpSpeedComparator(boolean asc, int idx) {
        this.asc = asc;
        this.idx = idx;
    }

    public int compare(Object o1, Object o2) {

        HttpSnap snap1 = (HttpSnap) o1;
        HttpSnap snap2 = (HttpSnap) o2;

        if (snap1.httpSpeed[idx] == snap2.httpSpeed[idx])
            return 0;
        else if (snap1.httpSpeed[idx] < snap2.httpSpeed[idx])
            return asc ? -1 : 1;
        else
            return asc ? 1 : -1;
    }
}
