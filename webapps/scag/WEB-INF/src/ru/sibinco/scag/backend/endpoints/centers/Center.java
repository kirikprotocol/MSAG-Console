/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.centers;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.Constants;

import java.io.PrintWriter;

/**
 * The <code>Center</code> class represents of
 * <p>SMS centers, HTTP and MMS scag<p/>
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

    public static final String IN_QUEUE_LIMIT  = "inQueueLimit";
    public static final String OUT_QUEUE_LIMIT  = "outQueueLimit";
    public static final String MAX_SMS_PER_SEC = "maxSmsPerSec";

    private String id = null;
    private byte type = SMPP;
    private int timeout = 0;
    private byte mode = MODE_TRX;
    private String host = null;
    private int port = 0;
    private String altHost;
    private int altPort;
    private boolean enabled = true;
    private boolean snmpTracking;
    private Provider provider;
    private int uid = -1;
    private String bindSystemId;
    private String bindPassword = "";
    private String systemType = "";
    private String transport = "SMPP";
    private String addressRange;
    private String connHostPort = "";
    private String connStatus = "unknow";
    private int inQueueLimit = 0;
    private int outQueueLimit = 0;
    private int maxSmsPerSec = 0;
    private Category logger = Category.getInstance(this.getClass());
    private String metaGroup = "";

    public Center(String id, int timeout,
                  byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled, final Provider provider,
                  final int uid, final String bindSystemId, final String bindPassword, final String systemType, final String addressRange) throws NullPointerException {
        if (null == id || bindSystemId == null)
            throw new NullPointerException("SMSC ID or bind Password or bind SystemId  is null");
        this.id = id;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
        this.provider = provider;
        this.uid = uid;
        this.bindSystemId = bindSystemId;
        this.bindPassword = bindPassword;
        this.systemType = systemType;
        this.addressRange = addressRange;
    }

    public Center(String id, int timeout, byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled, final Provider provider,
                  final int uid, final String bindSystemId, final String bindPassword, final String systemType,
                  final String addressRange, final int inQueueLimit, final int maxSmsPerSec)
                  throws NullPointerException {
        if (null == id || bindSystemId == null)
            throw new NullPointerException("SMSC ID or bind Password or bind SystemId  is null");
        this.id = id;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
        this.provider = provider;
        this.uid = uid;
        this.bindSystemId = bindSystemId;
        this.bindPassword = bindPassword;
        this.systemType = systemType;
        this.addressRange = addressRange;
        this.inQueueLimit = inQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
    }

    public Center(String id, int timeout, byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled, final Provider provider,
                  final int uid, final String bindSystemId, final String bindPassword, final String systemType,
                  final String addressRange, final int inQueueLimit, final int outQueueLimit,
                  final int maxSmsPerSec)
                  throws NullPointerException {
        if (null == id || bindSystemId == null)
            throw new NullPointerException("SMSC ID or bind Password or bind SystemId  is null");
        this.id = id;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
        this.provider = provider;
        this.uid = uid;
        this.bindSystemId = bindSystemId;
        this.bindPassword = bindPassword;
        this.systemType = systemType;
        this.addressRange = addressRange;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
    }

    public Center(String id, int timeout, byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled, final Provider provider,
                  final int uid, final String bindSystemId, final String bindPassword, final String systemType,
                  final String addressRange, final int inQueueLimit, final int outQueueLimit,
                  final int maxSmsPerSec, final String metaGroup)
                  throws NullPointerException {
        if (null == id || bindSystemId == null)
            throw new NullPointerException("SMSC ID or bind Password or bind SystemId  is null");
        this.id = id;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
        this.provider = provider;
        this.uid = uid;
        this.bindSystemId = bindSystemId;
        this.bindPassword = bindPassword;
        this.systemType = systemType;
        this.addressRange = addressRange;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
        this.metaGroup = metaGroup;
    }

    public Center(String id, int timeout, byte mode, String host, int port, String altHost,
                  int altPort, boolean enabled, final Provider provider,
                  final int uid, final String bindSystemId, final String bindPassword, final String systemType,
                  final String addressRange, final int inQueueLimit, final int outQueueLimit,
                  final int maxSmsPerSec, final String metaGroup, final boolean snmpTracking)
                  throws NullPointerException {
        if (null == id || bindSystemId == null)
            throw new NullPointerException("SMSC ID or bind Password or bind SystemId  is null");
        this.id = id;
        this.timeout = timeout;
        this.mode = mode;
        this.host = host;
        this.port = port;
        this.altHost = altHost;
        this.altPort = altPort;
        this.enabled = enabled;
        this.snmpTracking = snmpTracking;
        this.provider = provider;
        this.uid = uid;
        this.bindSystemId = bindSystemId;
        this.bindPassword = bindPassword;
        this.systemType = systemType;
        this.addressRange = addressRange;
        this.inQueueLimit = inQueueLimit;
        this.outQueueLimit = outQueueLimit;
        this.maxSmsPerSec = maxSmsPerSec;
        this.metaGroup = metaGroup;
    }

    public Center(final Element centersElement, final ProviderManager providerManager) throws NullPointerException {
        final NodeList list = centersElement.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            final Element paramElem = (Element) list.item(i);
            final String name = paramElem.getAttribute("name");
            final String value = paramElem.getAttribute("value");
            try {
                if ("systemId".equals(name)) {
                    id = StringEncoderDecoder.encode(value);
                } else if ("timeout".equals(name)) {
                    timeout = Integer.decode(value).intValue();
                } else if ("enabled".equals(name)) {
                    enabled = Boolean.valueOf(value).booleanValue();
                } else if ("snmpTracking".equals(name)) {
                    snmpTracking = Boolean.valueOf(value).booleanValue();
                } else if ("mode".equals(name)) {
                    mode = getMode(value);
                } else if ("host".equals(name)) {
                    host = value;
                } else if ("port".equals(name)) {
                    port = Integer.decode(value).intValue();
                } else if ("althost".equals(name)) {
                    altHost = value;
                } else if ("altport".equals(name)) {
                    altPort = Integer.decode(value).intValue();
                } else if ("uid".equals(name)) {
                    uid = Integer.decode(value).intValue();
                } else if ("providerId".equals(name)) {
                    provider = (Provider) providerManager.getProviders().get(Long.decode(value));
                } else if ("bindSystemId".equals(name)) {
                    bindSystemId = value;
                } else if ("bindPassword".equals(name)) {
                    bindPassword = value;
                } else if ("systemType".equals(name)) {
                    systemType = value;
                } else if ("addressRange".equals(name)){
                    addressRange = value;
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
//                logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
                logger.warn("Int parameter \"" + name + "\" misformatted: '" + value + "', skipped");
            }
        }

//        if (null == id || null == password)
//            throw new NullPointerException("SME System ID is null");
    }

    public Center(final Center center) {
        this.id = center.getId();
        this.timeout = center.getTimeout();
        this.mode = center.getMode();
        this.host = center.getHost();
        this.port = center.getPort();
        this.altHost = center.getAltHost();
        this.altPort = center.getAltPort();
        this.enabled = center.isEnabled();
        this.snmpTracking = center.isSnmpTracking();
        this.provider = center.getProvider();
        this.uid = center.getUid();
        this.addressRange = center.getAddressRange();
        this.inQueueLimit = center.getInQueueLimit();
        this.outQueueLimit = center.getOutQueueLimit();
        this.maxSmsPerSec = center.getMaxSmsPerSec();
        this.metaGroup = center.getMetaGroup();
    }

    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <" + Constants.SMSC_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </" + Constants.SMSC_RECORD_TAG + ">");
        return out;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
    }

    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("    <param name=\"systemId\"\tvalue=\"" + StringEncoderDecoder.encode(id) + "\"/>");
        out.println("    <param name=\"bindSystemId\"\tvalue=\"" + StringEncoderDecoder.encode(bindSystemId) + "\"/>");
        out.println("    <param name=\"bindPassword\"\tvalue=\"" + StringEncoderDecoder.encode(bindPassword) + "\"/>");
        out.println("    <param name=\"systemType\"\tvalue=\"" + StringEncoderDecoder.encode(systemType) + "\"/>");
        out.println("    <param name=\"timeout\"\tvalue=\"" + timeout + "\"/>");
        out.println("    <param name=\"mode\"\tvalue=\"" + getModeStr() + "\"/>");
        out.println("    <param name=\"host\"\tvalue=\"" + host + "\"/>");
        out.println("    <param name=\"port\"\tvalue=\"" + port + "\"/>");
        out.println("    <param name=\"althost\"\tvalue=\"" + altHost + "\"/>");
        out.println("    <param name=\"altport\"\tvalue=\"" + altPort + "\"/>");
        out.println("    <param name=\"enabled\"\tvalue=\"" + enabled + "\"/>");
        out.println("    <param name=\"snmpTracking\"\tvalue=\"" + snmpTracking + "\"/>");
        out.println("    <param name=\"uid\"\tvalue=\"" + uid + "\"/>");
        out.println("    <param name=\"providerId\"\tvalue=\"" + -1/*provider.getId()*/ + "\"/>");
        out.println("    <param name=\"addressRange\"\tvalue=\"" + addressRange.trim() + "\"/>");
        out.println("    <param name=\""+ IN_QUEUE_LIMIT +"\"\tvalue=\"" + intToString(inQueueLimit) + "\"/>");
        out.println("    <param name=\""+ OUT_QUEUE_LIMIT +"\"\tvalue=\"" + intToString(outQueueLimit) + "\"/>");
        out.println("    <param name=\""+ MAX_SMS_PER_SEC +"\"\tvalue=\"" + intToString(maxSmsPerSec) + "\"/>");
        out.println("    <param name=\"metaGroup\"\tvalue=\"" + StringEncoderDecoder.encode(metaGroup) + "\"/>");
        return out;
    }

    public String intToString(int number){
        return number==0?"-1":new Integer(number).toString();
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

    public boolean isSnmpTracking() {
        return snmpTracking;
    }

    public void setSnmpTracking(final boolean snmpTracking) {
        this.snmpTracking = snmpTracking;
    }

    public Provider getProvider() {
        return provider;
    }

    public void setProvider(final Provider provider) {
        this.provider = provider;
    }

    public String getTransport() {
        return transport;
    }

    public void setTransport(final String transport) {
        this.transport = transport;
    }

    public int getUid() {
        return uid;
    }

    public void setUid(final int uid) {
        this.uid = uid;
    }

    public String getProviderName() {
        if (null != provider)
            return provider.getName();
        else
            return null;
    }

    public String getBindSystemId() {
        return bindSystemId;
    }

    public void setBindSystemId(final String bindSystemId) {
        this.bindSystemId = bindSystemId;
    }

    public String getBindPassword() {
        return bindPassword;
    }

    public void setBindPassword(final String bindPassword) {
        this.bindPassword = bindPassword;
    }

    public String getSystemType() {
        return systemType;
    }

    public void setSystemType(final String systemType) {
        this.systemType = systemType;
    }

    public String getAddressRange() {
        return addressRange;
    }

    public void setAddressRange(final String addressRange) {
        this.addressRange = addressRange;
    }


    public String getConnStatus() {
        return connStatus;
    }

    public void setConnStatus(String connStatus) {
        this.connStatus = connStatus;
    }

    public String getConnHostPort() {
        return connHostPort;
    }

    public void setConnHostPort(String connHostPort) {
        this.connHostPort = connHostPort;
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

    public void setMetaGroup(String metaGroup) {
        this.metaGroup = metaGroup;
    }

}
