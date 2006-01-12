/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.snap;

import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.DataInputStream;

/**
 * The <code>SmppSnap</code> class represents
 * <p><p/>
 * Date: 07.12.2005
 * Time: 15:14:08
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppSnap extends SnapAbstract {


    public String smppId;
    public short[] smppSpeed = new short[COUNTERS];
    public short[] smppAvgSpeed = new short[COUNTERS];

    public SmppSnap() {
    }

    public SmppSnap(SmppSnap snap) {

        smppId = new String(snap.smppId);
        System.arraycopy(snap.smppSpeed, 0, smppSpeed, 0, COUNTERS);
        System.arraycopy(snap.smppAvgSpeed, 0, smppAvgSpeed, 0, COUNTERS);
    }

    public void init(SnapBufferReader in) throws IOException {

        smppId = in.readNetworkCFixedString(MAX_STR_LEN);
        for (int i = 0; i < COUNTERS; i++) {
            smppSpeed[i] = in.readNetworkShort();
            smppAvgSpeed[i] = in.readNetworkShort();
        }
    }

    public void write(DataOutputStream out) throws IOException {
        out.writeUTF(smppId);
        for (int i = 0; i < COUNTERS; i++) {
            out.writeShort(smppSpeed[i]);
            out.writeShort(smppAvgSpeed[i]);
        }
    }

    public void read(DataInputStream in) throws IOException {
        smppId = in.readUTF();
        for (int i = 0; i < COUNTERS; i++) {
            smppSpeed[i] = in.readShort();
            smppAvgSpeed[i] = in.readShort();
        }
    }
}

