/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.snap;

import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.DataInputStream;

/**
 * The <code>MmsSnap</code> class represents
 * <p><p/>
 * Date: 11.01.2006
 * Time: 12:44:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class MmsSnap extends SnapAbstract {

    public String mmsId;
    public short[] mmsSpeed = new short[COUNTERS];
    public short[] mmsAvgSpeed = new short[COUNTERS];

    public MmsSnap() {
    }

    public MmsSnap(MmsSnap snap) {
        mmsId = new String(snap.mmsId);
        System.arraycopy(snap.mmsSpeed, 0, mmsSpeed, 0, COUNTERS);
        System.arraycopy(snap.mmsAvgSpeed, 0, mmsAvgSpeed, 0, COUNTERS);
    }

    public void init(SnapBufferReader in) throws IOException {
        mmsId = in.readNetworkCFixedString(MAX_STR_LEN);
        for (int i = 0; i < COUNTERS; i++) {
            mmsSpeed[i] = in.readNetworkShort();
            mmsAvgSpeed[i] = in.readNetworkShort();
        }
    }

    public void write(DataOutputStream out) throws IOException {
        out.writeUTF(mmsId);
        for (int i = 0; i < COUNTERS; i++) {
            out.writeShort(mmsSpeed[i]);
            out.writeShort(mmsAvgSpeed[i]);
        }
    }

    public void read(DataInputStream in) throws IOException {
        mmsId = in.readUTF();
        for (int i = 0; i < COUNTERS; i++) {
            mmsSpeed[i] = in.readShort();
            mmsAvgSpeed[i] = in.readShort();
        }
    }
}

