/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.applet;


import ru.sibinco.scag.scmon.snap.SmppSnap;

import java.util.Comparator;

/**
 * The <code>SmppIdComparator</code> class represents
 * <p><p/>
 * Date: 12.12.2005
 * Time: 13:24:53
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppIdComparator implements Comparator {
    boolean asc;

    public SmppIdComparator(boolean asc) {
        this.asc = asc;
    }

    public int compare(Object o1, Object o2) {
        SmppSnap snap1 = (SmppSnap) o1;
        SmppSnap snap2 = (SmppSnap) o2;
        int rc = snap1.smppId.compareTo(snap2.smppId);
        return asc ? rc : -rc;
    }
}
