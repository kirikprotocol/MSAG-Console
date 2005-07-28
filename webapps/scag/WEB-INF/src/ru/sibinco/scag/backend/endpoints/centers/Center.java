/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.centers;

import org.w3c.dom.Element;

import java.io.PrintWriter;

/**
 * The <code>Center</code> class represents of
 * <p>SMS centers, WAP and MMS gateway<p/>
 * Date: 15.07.2005
 * Time: 12:43:26
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Center {

    public static final byte SMPP = 0;
    public static final byte SS7 = 1;

    private String id = null;
    private byte type = SMPP;

    public Center(final Element centersElement) throws NullPointerException {
        this.id = centersElement.getAttribute("uid").trim();
        this.type = "smpp".equals(centersElement.getAttribute("type")) ? SMPP : SS7;
    }

    public Center(final Center center) {
        this.id = center.getId();
        this.type = center.getType();
    }

    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <centersrecord type=\"" + getTypeStr() + "\" uid=\"" + id + "\">"); // ToDo add  correct values
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </centersrecord>");  //ToDo
        return out;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
    }


    protected PrintWriter storeBody(final PrintWriter out) {
//        out.println("    <param name=\"priority\"          value=\"" + priority + "\"/>");
//        out.println("    <param name=\"typeOfNumber\"      value=\"" + typeOfNumber + "\"/>"); //ToDo:

        return out;
    }

    private String getTypeStr
            () {
        switch (type) {
            case SMPP:
                return "smpp";
            case SS7:
                return "ss7";
            default:
                return "unknown";
        }
    }

    public boolean equals
            (final Object obj) {
        if (obj instanceof Center)
            return ((Center) obj).id.equals(id);
        else
            return super.equals(obj);
    }

    public int hashCode
            () {
        return id.hashCode();
    }

    public String getId
            () {
        return id;
    }

    public void setId
            (final String id) {
        this.id = id;
    }

    public byte getType
            () {
        return type;
    }

    public void setType
            (final byte type) {
        this.type = type;
    }
}
