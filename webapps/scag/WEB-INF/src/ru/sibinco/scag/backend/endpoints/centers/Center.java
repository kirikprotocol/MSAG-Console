/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.centers;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.apache.log4j.Category;

import java.io.PrintWriter;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;

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

    public static final byte MODE_TX = 1;
    public static final byte MODE_RX = 2;
    public static final byte MODE_TRX = 3;
    public static final byte MODE_UNKNOWN = 0;

    private String id = null;
    private byte type = SMPP;
    private String password = null;
    private int timeout = 0;
    private byte mode = MODE_TRX;
    private String host = null;
    private int port = 0;
    private String altHost;
    private int altPort;
    private boolean enabled = false;

    private Category logger = Category.getInstance(this.getClass());

    public Center(String id, String password, int timeout,
                  byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled) throws NullPointerException{
        if (null == id || null == password)
            throw new NullPointerException("SMSC ID or  password  is null");
        this.id = id;
        this.password = password;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
    }

    public Center(final Element centersElement) throws NullPointerException {
        final NodeList list = centersElement.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            final Element paramElem = (Element) list.item(i);
            final String name = paramElem.getAttribute("name");
            final String value = paramElem.getAttribute("value");
            try {
                if ("systemId".equals(name)) {
                    id = value;
                } else if ("password".equals(name)) {
                    password = value;
                } else if ("timeout".equals(name)) {
                    timeout = Integer.decode(value).intValue();
                } else if ("enabled".equals(name)) {
                    enabled = Boolean.valueOf(value).booleanValue();
                } else if ("mode".equals(name)) {
                    mode = getMode(value);
                } else if ("host".equals(name)) {
                    host = value;
                } else if ("port".equals(name)) {
                    port = Integer.decode(value).intValue();
                } else if ("altHost".equals(name)) {
                    altHost = value;
                } else if ("altPort".equals(name)) {
                    altPort = Integer.decode(value).intValue();
                }

            } catch (NumberFormatException e) {
                logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
            }
        }

        if (null == id || null == password)
            throw new NullPointerException("SME System ID is null");
    }

    public Center(final Center center) {
        this.id = center.getId();
        this.password = center.getPassword();
        this.timeout = center.getTimeout();
        this.mode = center.getMode();
        this.host = center.getHost();
        this.port = center.getPort();
        this.altHost = center.getAltHost();
        this.altPort = center.getAltPort();
        this.enabled = center.isEnabled();
    }

    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <smscrecord>");
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </smscrecord>");
        return out;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
    }


    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("    <param name=\"systemId\"   value=\"" + id + "\"/>");
        out.println("    <param name=\"password\"   value=\"" + StringEncoderDecoder.encode(password) + "\"/>");
        out.println("    <param name=\"timeout\"    value=\"" + timeout + "\"/>");
        out.println("    <param name=\"mode\"       value=\"" + getModeStr() + "\"/>");
        out.println("    <param name=\"host\"       value=\"" + host + "\"/>");
        out.println("    <param name=\"port\"       value=\"" + port + "\"/>");
        out.println("    <param name=\"althost\"    value=\"" + altHost + "\"/>");
        out.println("    <param name=\"altport\"    value=\"" + altPort + "\"/>");
        out.println("    <param name=\"enabled\"    value=\"" + enabled + "\"/>");

        return out;
    }

    public String getModeStr() {
        switch (mode) {
            case MODE_TX:
                return "tx";
            case MODE_RX:
                return "rx";
            case MODE_TRX:
                return "trx";
            default:
                return "unknown";
        }
    }

     private static byte getMode(final String modeStr) {
        if ("tx".equalsIgnoreCase(modeStr))
            return MODE_TX;
        else if ("rx".equalsIgnoreCase(modeStr))
            return MODE_RX;
        else if ("trx".equalsIgnoreCase(modeStr))
            return MODE_TRX;
        return MODE_UNKNOWN;
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

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public String getHost() {
        return host;
    }

    public void setHost(final String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(final int port) {
        this.port = port;
    }

    public String getAltHost() {
        return altHost;
    }

    public void setAltHost(final String altHost) {
        this.altHost = altHost;
    }

    public int getAltPort() {
        return altPort;
    }

    public void setAltPort(final int altPort) {
        this.altPort = altPort;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }
}
