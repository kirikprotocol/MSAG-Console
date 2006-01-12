/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.SvcSnap;
import ru.sibinco.scag.svcmon.snap.SmppSnap;

/**
 * The <code>SnapSmppHistory</code> class represents
 * <p><p/>
 * Date: 08.12.2005
 * Time: 17:19:17
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SnapSmppHistory {
    public static final int MAX_HISTORY_LENGTH = 2000;
    SvcSnap[] snaps = new SvcSnap[MAX_HISTORY_LENGTH];
    int headSmpp = 0;
    public int countSmmp = 0;
    SmppSnap[] smppSnaps = new SmppSnap[MAX_HISTORY_LENGTH];

    String currentSmpp = null;

    public void addSnap(SvcSnap svcSnap) {

        if (countSmmp < MAX_HISTORY_LENGTH) {
            snaps[countSmmp] = new SvcSnap(svcSnap);
            if (currentSmpp != null) {
                smppSnaps[countSmmp] = new SmppSnap(findSmppSnap(svcSnap));
            }
            countSmmp++;
        } else {
            snaps[headSmpp] = new SvcSnap(svcSnap);
            if (currentSmpp != null) {
                smppSnaps[headSmpp] = new SmppSnap(findSmppSnap(svcSnap));
            }
            headSmpp++;
            if (headSmpp == MAX_HISTORY_LENGTH) headSmpp = 0;
        }

    }

    SmppSnap findSmppSnap(SvcSnap snap) {
        for (int i = 0; i < snap.smppCount; i++) {
            if (snap.smppSnaps[i].smppId.equals(currentSmpp)) {
                return snap.smppSnaps[i];
            }
        }
        return null;
    }


    public String getCurrentSmpp() {
        return currentSmpp;
    }

    public void setCurrentSmpp(String smpp) {
        if (currentSmpp == null || !currentSmpp.equals(smpp)) {
            currentSmpp = new String(smpp);
            for (int i = 0; i < countSmmp; i++) {
                smppSnaps[i] = new SmppSnap(findSmppSnap(snaps[i]));
            }
        }
    }


    int smmpIdx;

    public SmppSnap getSmppLast() {
        if (countSmmp == 0) return null;
        if (countSmmp == MAX_HISTORY_LENGTH) {
            if (headSmpp == 0)
                smmpIdx = countSmmp - 1;
            else
                smmpIdx = headSmpp - 1;
        } else {
            smmpIdx = countSmmp - 1;
        }
        return smppSnaps[smmpIdx];
    }


    public SmppSnap getPrevSmpp() {
        if (countSmmp == 0) return null;
        smmpIdx--;
        if (smmpIdx == -1) smmpIdx = countSmmp - 1;
        return smppSnaps[smmpIdx];
    }
}

