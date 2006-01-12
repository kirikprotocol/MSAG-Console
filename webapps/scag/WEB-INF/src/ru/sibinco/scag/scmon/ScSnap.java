/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon;

import ru.sibinco.lib.backend.util.SnapBufferReader;
import ru.sibinco.scag.scmon.snap.MmsSnap;
import ru.sibinco.scag.scmon.snap.HttpSnap;
import ru.sibinco.scag.scmon.snap.SmppSnap;

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.util.Comparator;
import java.util.Arrays;

public class ScSnap {

    public int smppCount;
    public SmppSnap smppSnaps[];
    public int httpCount;
    public HttpSnap httpSnaps[];
    public int mmsCount;
    public MmsSnap mmsSnaps[];


    public ScSnap() {
        smppCount = 0;
        smppSnaps = null;
        httpCount = 0;
        httpSnaps = null;
        mmsCount = 0;
        mmsSnaps = null;

    }

    public ScSnap(ScSnap snap) {
        smppCount = snap.smppCount;
        smppSnaps = new SmppSnap[smppCount];
        for (int i = 0; i < smppCount; i++) {
            smppSnaps[i] = new SmppSnap(snap.smppSnaps[i]);
        }

        httpCount = snap.httpCount;
        httpSnaps = new HttpSnap[httpCount];
        for (int i = 0; i < httpCount; i++) {
            httpSnaps[i] = new HttpSnap(snap.httpSnaps[i]);
        }

        mmsCount = snap.mmsCount;
        mmsSnaps = new MmsSnap[mmsCount];
        for (int i = 0; i < mmsCount; i++) {
            mmsSnaps[i] = new MmsSnap(snap.mmsSnaps[i]);
        }
    }

    public void init(SnapBufferReader in) throws IOException {

        smppCount = ((int) in.readNetworkShort()) & 0xFFFF;
        if (smppSnaps == null || smppSnaps.length < smppCount) {
            smppSnaps = new SmppSnap[smppCount];
            for (int i = 0; i < smppCount; i++) {
                smppSnaps[i] = new SmppSnap();
            }
        }
        for (int i = 0; i < smppCount; i++) {
            smppSnaps[i].init(in);
        }

        httpCount = ((int) in.readNetworkShort()) & 0xFFFF;
        if (httpSnaps == null || httpSnaps.length < httpCount) {
            httpSnaps = new HttpSnap[httpCount];
            for (int i = 0; i < httpCount; i++) {
                httpSnaps[i] = new HttpSnap();
            }
        }

        for (int i = 0; i < httpCount; i++) {
            httpSnaps[i].init(in);
        }

        mmsCount = ((int) in.readNetworkShort()) & 0xFFFF;
        if (mmsSnaps == null || mmsSnaps.length < mmsCount) {
            mmsSnaps = new MmsSnap[mmsCount];
            for (int i = 0; i < mmsCount; i++) {
                mmsSnaps[i] = new MmsSnap();
            }
        }
        for (int i = 0; i < mmsCount; i++) {
            mmsSnaps[i].init(in);
        }
    }

    public void write(DataOutputStream out) throws IOException {
        out.writeInt(smppCount);
        for (int i = 0; i < smppCount; i++) {
            smppSnaps[i].write(out);
        }
        out.writeInt(httpCount);
        for (int i = 0; i < httpCount; i++) {
            httpSnaps[i].write(out);
        }
        out.writeInt(mmsCount);
        for (int i = 0; i < mmsCount; i++) {
            mmsSnaps[i].write(out);
        }
    }

    public void read(DataInputStream in) throws IOException {

        smppCount = in.readInt();
        if (smppSnaps == null || smppSnaps.length < smppCount) {
            smppSnaps = new SmppSnap[smppCount];
            for (int i = 0; i < smppCount; i++) {
                smppSnaps[i] = new SmppSnap();
            }
        }
        for (int i = 0; i < smppCount; i++) {
            smppSnaps[i].read(in);
        }

        httpCount = in.readInt();
        if (httpSnaps == null || httpSnaps.length < httpCount) {
            httpSnaps = new HttpSnap[httpCount];
            for (int i = 0; i < httpCount; i++) {
                httpSnaps[i] = new HttpSnap();
            }
        }
        for (int i = 0; i < httpCount; i++) {
            httpSnaps[i].read(in);
        }

        mmsCount = in.readInt();
        if (mmsSnaps == null || mmsSnaps.length < mmsCount) {
            mmsSnaps = new MmsSnap[mmsCount];
            for (int i = 0; i < mmsCount; i++) {
                mmsSnaps[i] = new MmsSnap();
            }
        }
        for (int i = 0; i < mmsCount; i++) {
            mmsSnaps[i].read(in);
        }
    }

    public void sortSmppSnaps(Comparator comparator) {
        Arrays.sort(smppSnaps, comparator);
    }

    public void sortHttpSnaps(Comparator comparator) {
        Arrays.sort(httpSnaps, comparator);
    }

    public void sortMmsSnaps(Comparator comparator) {
        Arrays.sort(mmsSnaps, comparator);
    }
}

