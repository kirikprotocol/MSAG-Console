/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.snap;

import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.DataInputStream;

/**
 * The <code>HttpSnap</code> class represents
 * <p><p/>
 * Date: 07.12.2005
 * Time: 16:43:35
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpSnap extends SnapAbstract {

    public static final int COUNTERS = 6;

    public static final int REQUEST_INDEX = 0;
    public static final int REQUEST_REJECTED_INDEX = 1;
    public static final int RESPONSE_INDEX = 2;
    public static final int RESPONSE_REJECTED_INDEX = 3;
    public static final int DELIVERED_INDEX = 4;
    public static final int FAILED_INDEX = 5;

    
    public String httpId;
    public short[] httpSpeed = new short[COUNTERS];
    public short[] httpAvgSpeed = new short[COUNTERS];

    public HttpSnap() {
    }

    public HttpSnap(HttpSnap snap) {
        httpId = new String(snap.httpId);
        System.arraycopy(snap.httpSpeed, 0, httpSpeed, 0, COUNTERS);
        System.arraycopy(snap.httpAvgSpeed, 0, httpAvgSpeed, 0, COUNTERS);
    }

    public void init(SnapBufferReader in) throws IOException {
        httpId = in.readNetworkCFixedString(MAX_STR_LEN);
        for (int i = 0; i < COUNTERS; i++) {
            httpSpeed[i] = in.readNetworkShort();
            httpAvgSpeed[i] = in.readNetworkShort();
        }
    }

    public void write(DataOutputStream out) throws IOException {
        out.writeUTF(httpId);

        for (int i = 0; i < COUNTERS; i++) {
            out.writeShort(httpSpeed[i]);
            out.writeShort(httpAvgSpeed[i]);
        }
    }

    public void read(DataInputStream in) throws IOException {
        httpId = in.readUTF();
        for (int i = 0; i < COUNTERS; i++) {
            httpSpeed[i] = in.readShort();
            httpAvgSpeed[i] = in.readShort();
        }
    }
}
