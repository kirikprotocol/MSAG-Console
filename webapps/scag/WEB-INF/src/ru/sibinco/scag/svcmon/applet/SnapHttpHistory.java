/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.SvcSnap;
import ru.sibinco.scag.svcmon.snap.HttpSnap;

/**
 * The <code>SnapHttpHistory</code> class represents
 * <p><p/>
 * Date: 26.12.2005
 * Time: 13:07:36
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SnapHttpHistory {

    public static final int MAX_HISTORY_LENGTH = 2000;
    SvcSnap[] snaps = new SvcSnap[MAX_HISTORY_LENGTH];
    int headHttp = 0;
    public int countHttp = 0;
    HttpSnap[] httpSnaps = new HttpSnap[MAX_HISTORY_LENGTH];

    String currentHttp = null;

    public void addSnap(SvcSnap svcSnap) {

        if (countHttp < MAX_HISTORY_LENGTH) {
            snaps[countHttp] = new SvcSnap(svcSnap);
            if (currentHttp != null) {
                httpSnaps[countHttp] = new HttpSnap(findHttpSnap(svcSnap));
            }
            countHttp++;
        } else {
            snaps[headHttp] = new SvcSnap(svcSnap);
            if (currentHttp != null) {
                httpSnaps[headHttp] = new HttpSnap(findHttpSnap(svcSnap));
            }
            headHttp++;
            if (headHttp == MAX_HISTORY_LENGTH) headHttp = 0;
        }

    }

    HttpSnap findHttpSnap(SvcSnap snap) {
        for (int i = 0; i < snap.httpCount; i++) {
            if (snap.httpSnaps[i].httpId.equals(currentHttp)) {
                return snap.httpSnaps[i];
            }
        }
        return null;
    }


    public String getCurrentHttp() {
        return currentHttp;
    }

    public void setCurrentHttp(String http) {
        if (currentHttp == null || !currentHttp.equals(http)) {
            currentHttp = new String(http);
            for (int i = 0; i < countHttp; i++) {
                httpSnaps[i] = new HttpSnap(findHttpSnap(snaps[i]));
            }
        }
    }


    int httpIdx;

    public HttpSnap getHttpLast() {
        if (countHttp == 0) return null;
        if (countHttp == MAX_HISTORY_LENGTH) {
            if (headHttp == 0)
                httpIdx = countHttp - 1;
            else
                httpIdx = headHttp - 1;
        } else {
            httpIdx = countHttp - 1;
        }
        return httpSnaps[httpIdx];
    }


    public HttpSnap getPrevHttp() {
        if (countHttp == 0) return null;
        httpIdx--;
        if (httpIdx == -1) httpIdx = countHttp - 1;
        return httpSnaps[httpIdx];
    }
}

