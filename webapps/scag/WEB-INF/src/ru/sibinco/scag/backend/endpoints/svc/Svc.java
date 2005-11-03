/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.svc;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.apache.log4j.Category;

import java.io.PrintWriter;

import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;

/**
 * The <code>Svc</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:27:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Svc {

    public static final byte SMPP = 0;
    public static final byte SS7 = 1;
    public static final byte WAP = 2;
    public static final byte MMS = 3;

    public static final byte MODE_TX = 1;
    public static final byte MODE_RX = 2;
    public static final byte MODE_TRX = 3;
    public static final byte MODE_UNKNOWN = 0;

    private String id = null;
    private String password = null;
    private int timeout = 0;
    private boolean enabled = false;
    private byte mode = MODE_TRX;
    private Provider provider;
    private String transport = "SMPP";
    
    private byte type = SMPP;

    private SmeStatus status = null;

    private Category logger = Category.getInstance(this.getClass());

    public Svc() {
    }

    public Svc(final String id, final String password, final int timeout,
               final boolean enabled, final byte mode, final Provider provider) throws NullPointerException{
        if (null == id || null == password)
            throw new NullPointerException("SME ID or  password  is null");
        this.id = id.trim();
        this.password = password;
        this.timeout = timeout;
        this.enabled = enabled;
        this.mode = mode;
        this.provider = provider;
    }

    public Svc(final Element svcElement, final ProviderManager providerManager) throws NullPointerException {
        final NodeList list = svcElement.getElementsByTagName("param");
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
                } else if ("providerId".equals(name)) {
                   provider = (Provider) providerManager.getProviders().get(Long.decode(value));
                }
            } catch (NumberFormatException e) {
                logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
            }
        }

        if (null == id || null == password)
            throw new NullPointerException("SME System ID is null");
    }

    public Svc(final Svc svc) {
        this.id = svc.getId();
        this.password = svc.getPassword();
        this.timeout = svc.getTimeout();
        this.enabled = svc.isEnabled();
        this.mode = svc.getMode();
    }


    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <smerecord>");
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </smerecord>");
        return out;
    }

    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("    <param name=\"systemId\"         value=\"" + StringEncoderDecoder.encode(id) + "\"/>");
        out.println("    <param name=\"password\"         value=\"" + StringEncoderDecoder.encode(password) + "\"/>");
        out.println("    <param name=\"timeout\"          value=\"" + timeout + "\"/>");
        out.println("    <param name=\"mode\"             value=\"" + getModeStr() + "\"/>");
        out.println("    <param name=\"enabled\"          value=\"" + enabled + "\"/>");
        out.println("    <param name=\"providerId\"       value=\"" + provider.getId() + "\"/>");

        return out;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
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

    public boolean equals
            (final Object obj) {
        if (obj instanceof Svc)
            return ((Svc) obj).id.equals(id);
        else
            return super.equals(obj);
    }

    public int hashCode() {
        return id.hashCode();
    }

    public void update(final Svc newSvc){
        id = newSvc.getId();
        password = newSvc.getPassword();
        timeout = newSvc.getTimeout();
        enabled = newSvc.isEnabled();
        mode = newSvc.getMode();
    }


    public String getId() {
        return id;
    }

    public void setId(final String id) {
        this.id = id;
    }

    public byte getType() {
        return type;
    }

    public void setType(final byte type) {
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

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public SmeStatus getStatus() {
        return status;
    }

    public void setStatus(final SmeStatus status) {
        this.status = status;
    }

    public Provider getProvider() {
        return provider;
    }

    public void setProvider(final Provider provider) {
        this.provider = provider;
    }

    public String getProviderName() {
        if (null != provider)
            return provider.getName();
        else
            return null;
    }

    public String getTransport() {
        return transport;
    }

    public void setTransport(String transport) {
        this.transport = transport;
    }


}


