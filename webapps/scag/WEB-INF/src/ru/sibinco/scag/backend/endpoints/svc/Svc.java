/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.svc;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.Constants;

import java.io.PrintWriter;

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
    public static final byte HTTP = 2;
    public static final byte MMS = 3;

    public static final byte MODE_TX = 1;
    public static final byte MODE_RX = 2;
    public static final byte MODE_TRX = 3;
    public static final byte MODE_UNKNOWN = 0;

    public static final String IN_QUEUE_LIMIT  = "inQueueLimit";
    public static final String OUT_QUEUE_LIMIT  = "outQueueLimit";
    public static final String MAX_SMS_PER_SEC = "maxSmsPerSec";

    private String id = null;
    private String password = "";
    private int timeout = 0;
    private boolean enabled = true;
    private boolean snmpTracking;
    private byte mode = MODE_TRX;
    private Provider provider;
    private String transport = "SMPP";
    private String connHost = "";
    private String connStatus = "unknow";
    private int inQueueLimit = 0;
    private int outQueueLimit = 0;
    private int maxSmsPerSec = 0;
    private String metaGroup = "";

    private byte type = SMPP;

    private Category logger = Category.getInstance(this.getClass());

    public Svc() {
    }

//    public Svc(final String id, final String password, final int timeout,
//               final boolean enabled, final byte mode, final Provider provider) throws NullPointerException{
//        if (null == id)
//            throw new NullPointerException("SME ID or  password  is null");
//        this.id = id.trim();
//        this.password = password;
//        this.timeout = timeout;
//        this.enabled = enabled;
//        this.mode = mode;
//        this.provider = provider;
//    }

    public Svc(final String id, final String password, final int timeout,
               final boolean enabled, final byte mode, final Provider provider,
               final int inQueueLimit, final int maxSmsPerSec)
               throws NullPointerException
    {
        if (null == id)
            throw new NullPointerException("SME ID or  password  is null");
        this.id = id.trim();
        this.password = password;
        this.timeout = timeout;
        this.enabled = enabled;
        this.mode = mode;
        this.provider = provider;
        this.inQueueLimit = inQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
    }

    public Svc(final String id, final String password, final int timeout,
               final boolean enabled, final byte mode, final Provider provider,
               final int inQueueLimit, final int outQueueLimit, final int maxSmsPerSec)
               throws NullPointerException
    {
        if (null == id)
            throw new NullPointerException("SME ID or  password  is null");
        this.id = id.trim();
        this.password = password;
        this.timeout = timeout;
        this.enabled = enabled;
        this.mode = mode;
        this.provider = provider;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
    }

        public Svc(final String id, final String password, final int timeout,
               final boolean enabled, final byte mode, final Provider provider,
               final int inQueueLimit, final int outQueueLimit, final int maxSmsPerSec, final String metaGroup)
               throws NullPointerException
    {
        if (null == id)
            throw new NullPointerException("SME ID or  password  is null");
        this.id = id.trim();
        this.password = password;
        this.timeout = timeout;
        this.enabled = enabled;
        this.mode = mode;
        this.provider = provider;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
        this.metaGroup = metaGroup;
    }

        public Svc(final String id, final String password, final int timeout,
               final boolean enabled, final byte mode, final Provider provider,
               final int inQueueLimit, final int outQueueLimit, final int maxSmsPerSec,
               final String metaGroup, final boolean snmpTracking)
               throws NullPointerException
    {
        if (null == id)
            throw new NullPointerException("SME ID or  password  is null");
        this.id = id.trim();
        this.password = password;
        this.timeout = timeout;
        this.enabled = enabled;
        this.snmpTracking = snmpTracking;
        this.mode = mode;
        this.provider = provider;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
        this.metaGroup = metaGroup;
    }

    public Svc(final Element svcElement, final ProviderManager providerManager)
            throws NullPointerException {
        final NodeList list = svcElement.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            final Element paramElem = (Element) list.item(i);
            final String name = paramElem.getAttribute("name");
            final String value = paramElem.getAttribute("value");
            try {
                if ("systemId".equals(name)) {
                    id = StringEncoderDecoder.encode(value);
                } else if ("password".equals(name)) {
                    password = value;
                } else if ("timeout".equals(name)) {
                    timeout = Integer.decode(value).intValue();
                } else if ("enabled".equals(name)) {
                    enabled = Boolean.valueOf(value).booleanValue();
                } else if ("snmpTracking".equals(name)) {
                    snmpTracking = Boolean.valueOf(value).booleanValue();
                } else if ("mode".equals(name)) {
                    mode = getMode(value);
                } else if ("providerId".equals(name)) {
                   provider = (Provider) providerManager.getProviders().get(Long.decode(value));
                } else if (IN_QUEUE_LIMIT.equals(name)) {
                   inQueueLimit = Integer.decode(value).intValue();
                } else if (OUT_QUEUE_LIMIT.equals(name)) {
                   outQueueLimit = Integer.decode(value).intValue();
                } else if (MAX_SMS_PER_SEC.equals(name)) {
                    maxSmsPerSec = Integer.decode(value).intValue();
                } else if ("metaGroup".equals(name)) {
                    metaGroup = StringEncoderDecoder.encode(value);
                }
            } catch (NumberFormatException e) {
                logger.warn("Int parameter \"" + name + "\" misformatted: '" + value + "', skipped");
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
        this.snmpTracking = svc.isSnmpTracking();
        this.mode = svc.getMode();
        this.inQueueLimit = svc.getInQueueLimit();
        this.outQueueLimit = svc.getOutQueueLimit();
        this.maxSmsPerSec = svc.getMaxSmsPerSec();
        this.metaGroup = svc.getMetaGroup();
    }


    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <" + Constants.SME_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </" + Constants.SME_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("    <param name=\"systemId\"\tvalue=\"" + StringEncoderDecoder.encode(id) + "\"/>");
        out.println("    <param name=\"password\"\tvalue=\"" + StringEncoderDecoder.encode(password) + "\"/>");
        out.println("    <param name=\"timeout\"\tvalue=\"" + timeout + "\"/>");
        out.println("    <param name=\"mode\"\t\tvalue=\"" + getModeStr() + "\"/>");
        out.println("    <param name=\"enabled\"\tvalue=\"" + enabled + "\"/>");
        out.println("    <param name=\"snmpTracking\"\tvalue=\"" + snmpTracking + "\"/>");
//        out.println("    <param name=\"providerId\"\tvalue=\"" + -1/*provider.getId()*/ + "\"/>");
        out.println("    <param name=\"" + IN_QUEUE_LIMIT +  "\"\tvalue=\"" + intToString(getInQueueLimit()) + "\"/>");
        out.println("    <param name=\"" + OUT_QUEUE_LIMIT + "\"\tvalue=\"" + intToString(getOutQueueLimit()) + "\"/>");
        out.println("    <param name=\"" + MAX_SMS_PER_SEC + "\"\tvalue=\"" + intToString(getMaxSmsPerSec()) + "\"/>");
        out.println("    <param name=\"metaGroup\"\tvalue=\"" + StringEncoderDecoder.encode(metaGroup) + "\"/>");
        return out;
    }

    public String intToString(int number){
        return number==0?"-1":new Integer(number).toString();
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter( storeBody( storeHeader(out) ) );
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
        snmpTracking = newSvc.isSnmpTracking();
        mode = newSvc.getMode();
        inQueueLimit =  newSvc.getInQueueLimit();
        outQueueLimit = newSvc.getOutQueueLimit();
        maxSmsPerSec = newSvc.getMaxSmsPerSec();
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

    public boolean isSnmpTracking() {
        return snmpTracking;
    }

    public void setSnmpTracking(final boolean snmpTracking) {
        this.snmpTracking = snmpTracking;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
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

    public String getConnHost() {
        return connHost;
    }

    public void setConnHost(String connHost) {
        this.connHost = connHost;
    }

    public String getConnStatus() {
        return connStatus;
    }

    public void setConnStatus(String connStatus) {
        this.connStatus = connStatus;
    }

    public void setInQueueLimit(int inQueueLimit) {
        this.inQueueLimit = inQueueLimit;
    }

    public void setOutQueueLimit(int outQueueLimit) {
        this.outQueueLimit = outQueueLimit;
    }

    public void setMaxSmsPerSec(int maxSmsPerSec) {
        this.maxSmsPerSec = maxSmsPerSec;
    }

    public int getInQueueLimit() {
        return inQueueLimit;
    }

    public int getOutQueueLimit() {
        return outQueueLimit;
    }

    public int getMaxSmsPerSec() {
        return maxSmsPerSec;
    }

    public String getMetaGroup() {
        return metaGroup;
    }

    public void setMetaGroup(String metaEndpoint) {
        this.metaGroup = metaEndpoint;
    }


}


