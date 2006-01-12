/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.snap;

import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.DataInputStream;

/**
 * The <code>SnapAbstract</code> class represents
 * <p><p/>
 * Date: 07.12.2005
 * Time: 16:46:11
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public abstract class SnapAbstract {

    public static final int COUNTERS = 5;

    public static final int ACCEPTED_INDEX = 0;
    public static final int REJECTED_INDEX = 1;
    public static final int DELIVERED_INDEX = 2;
    public static final int GW_REJECTED_INDEX = 3;
    public static final int FAILED_INDEX = 4;


    static final int MAX_STR_LEN = 16;

    public abstract void init(SnapBufferReader in) throws IOException;

    public abstract void write(DataOutputStream out) throws IOException;

    public abstract void read(DataInputStream in) throws IOException;
}
