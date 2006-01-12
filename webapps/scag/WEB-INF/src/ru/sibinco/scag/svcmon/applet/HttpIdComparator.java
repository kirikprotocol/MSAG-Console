/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.snap.HttpSnap;

import java.util.Comparator;

/**
 * The <code>HttpIdComparator</code> class represents
 * <p><p/>
 * Date: 26.12.2005
 * Time: 17:44:47
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpIdComparator implements Comparator {
    boolean asc;

    public HttpIdComparator(boolean asc) {
        this.asc = asc;
    }

    public int compare(Object o1, Object o2) {
        HttpSnap snap1 = (HttpSnap) o1;
        HttpSnap snap2 = (HttpSnap) o2;
        int rc = snap1.httpId.compareTo(snap2.httpId);
        return asc ? rc : -rc;
    }
}