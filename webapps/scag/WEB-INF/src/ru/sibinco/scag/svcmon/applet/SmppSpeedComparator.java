/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.snap.SmppSnap;

import java.util.Comparator;

/**
 * The <code>SmppSpeedComparator</code> class represents
 * <p><p/>
 * Date: 09.12.2005
 * Time: 13:31:11
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppSpeedComparator implements Comparator {

    int idx;
    boolean asc;

    public SmppSpeedComparator(boolean asc, int idx) {
        this.asc = asc;
        this.idx = idx;
    }

    public int compare(Object o1, Object o2) {

        SmppSnap snap1 = (SmppSnap) o1;
        SmppSnap snap2 = (SmppSnap) o2;

        if (snap1.smppSpeed[idx] == snap2.smppSpeed[idx])
            return 0;
        else if (snap1.smppSpeed[idx] < snap2.smppSpeed[idx])
            return asc ? -1 : 1;
        else
            return asc ? 1 : -1;
    }
}
