/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.services;

import org.w3c.dom.Element;

import java.io.PrintWriter;

/**
 * The <code>Services</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:27:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Services {

    public static final byte SMPP = 0;
    public static final byte SS7 = 1;

    private String id = null;
    private byte type = SMPP;


    public Services(final Element serviceElement) throws NullPointerException {
        this.id = serviceElement.getAttribute("uid").trim();
        this.type = "smpp".equals(serviceElement.getAttribute("type")) ? SMPP : SS7;
    }

    public Services(final Services service) {
        this.id = service.getId();
        this.type = service.getType();
    }

    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <servicesrecord type=\"" + getTypeStr() + "\" uid=\"" + id + "\">"); // ToDo add  correct values
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </servicesrecord>");  //ToDo
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
        if (obj instanceof Services)
            return ((Services) obj).id.equals(id);
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


