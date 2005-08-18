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
    private int priority = 0;
    private byte type = SMPP;
    private int typeOfNumber = 0;
    private int numberingPlan = 0;
    private int interfaceVersion = 0;
    private String systemType = null;
    private String password = null;
    private String addrRange = null;
    private int smeN = 0;
    private boolean wantAlias = false;
    private int timeout = 0;
    private boolean forceDC = false;
    private String receiptSchemeName = null;
    private boolean disabled = false;
    private byte mode = MODE_TRX;
    private int proclimit = 0;
    private int schedlimit = 0;

    private SmeStatus status = null;

    private Category logger = Category.getInstance(this.getClass());

    public Svc(final String id, final int priority, final byte type, final int typeOfNumber,
               final int numberingPlan, final int interfaceVersion, final String systemType,
               final String password, final String addrRange, final int smeN, final boolean wantAlias,
               final int timeout, final boolean forceDC, final String receiptSchemeName,
               final boolean disabled, final byte mode, final int proclimit, final int schedlimit) throws NullPointerException {
        if (null == id || null == systemType || null == password || null == addrRange)
            throw new NullPointerException("SME ID or system type or password or address range is null");

        this.id = id.trim();
        this.priority = priority;
        this.type = type;
        this.typeOfNumber = typeOfNumber;
        this.numberingPlan = numberingPlan;
        this.interfaceVersion = interfaceVersion;
        this.systemType = systemType.trim();
        this.password = password.trim();
        this.addrRange = addrRange.trim();
        this.smeN = smeN;
        this.wantAlias = wantAlias;
        this.timeout = timeout;
        this.forceDC = forceDC;
        if (null == this.receiptSchemeName)
            this.receiptSchemeName = "default";
        else
            this.receiptSchemeName = receiptSchemeName.trim();
        this.disabled = disabled;
        this.mode = mode;
        this.proclimit = proclimit;
        this.schedlimit = schedlimit;
    }


    public Svc(final Element svcElement) throws NullPointerException {
        this.receiptSchemeName = "default";
        this.id = svcElement.getAttribute("uid").trim();
        this.type = "smpp".equals(svcElement.getAttribute("type")) ? SMPP : SS7;
        final NodeList list = svcElement.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            final Element paramElem = (Element) list.item(i);
            final String name = paramElem.getAttribute("name");
            final String value = paramElem.getAttribute("value");
            try {
                if ("typeOfNumber".equals(name)) {
                    typeOfNumber = Integer.decode(value).intValue();
                } else if ("priority".equals(name)) {
                    priority = Integer.decode(value).intValue();
                } else if ("numberingPlan".equals(name)) {
                    numberingPlan = Integer.decode(value).intValue();
                } else if ("interfaceVersion".equals(name)) {
                    interfaceVersion = Integer.decode(value).intValue();
                } else if ("systemType".equals(name)) {
                    systemType = value;
                } else if ("password".equals(name)) {
                    password = value;
                } else if ("addrRange".equals(name)) {
                    addrRange = value;
                } else if ("smeN".equals(name)) {
                    smeN = Integer.decode(value).intValue();
                } else if ("wantAlias".equals(name)) {
                    wantAlias = "yes".equalsIgnoreCase(value) || "true".equalsIgnoreCase(value);
                } else if ("forceDC".equals(name)) {
                    forceDC = Boolean.valueOf(value).booleanValue();
                } else if ("timeout".equals(name)) {
                    timeout = Integer.decode(value).intValue();
                } else if ("receiptSchemeName".equals(name)) {
                    receiptSchemeName = value;
                } else if ("disabled".equals(name)) {
                    disabled = Boolean.valueOf(value).booleanValue();
                } else if ("mode".equals(name)) {
                    mode = getMode(value);
                } else if ("proclimit".equals(name)) {
                    proclimit = Integer.decode(value).intValue();
                } else if ("schedlimit".equals(name)) {
                    schedlimit = Integer.decode(value).intValue();
                }
            } catch (NumberFormatException e) {
                logger.error("Int parameter \"" + name + "\" misformatted: " + value + ", skipped", e);
            }
        }

        if (null == id || null == systemType || null == password || null == addrRange)
            throw new NullPointerException("SME System ID is null");
    }

    public Svc(final Svc svc) {
        this.id = svc.getId();
        this.priority = svc.getPriority();
        this.type = svc.getType();
        this.typeOfNumber = svc.getTypeOfNumber();
        this.numberingPlan = svc.getNumberingPlan();
        this.interfaceVersion = svc.getInterfaceVersion();
        this.systemType = svc.getSystemType();
        this.password = svc.getPassword();
        this.addrRange = svc.getAddrRange();
        this.smeN = svc.getSmeN();
        this.wantAlias = svc.isWantAlias();
        this.timeout = svc.getTimeout();
        this.forceDC = svc.isForceDC();
        this.receiptSchemeName = svc.getReceiptSchemeName();
        this.disabled = svc.isDisabled();
        this.mode = svc.getMode();
        this.receiptSchemeName = svc.getReceiptSchemeName();
        this.proclimit = svc.getProclimit();
        this.schedlimit = svc.getSchedlimit();
    }


    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <smerecord type=\"" + getTypeStr() + "\" uid=\"" + id + "\">"); // ToDo add  correct values
        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </smerecord>");  //ToDo
        return out;
    }

    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("    <param name=\"priority\"          value=\"" + priority + "\"/>");
        out.println("    <param name=\"typeOfNumber\"      value=\"" + typeOfNumber + "\"/>");
        out.println("    <param name=\"numberingPlan\"     value=\"" + numberingPlan + "\"/>");
        out.println("    <param name=\"interfaceVersion\"  value=\"" + interfaceVersion + "\"/>");
        out.println("    <param name=\"systemType\"        value=\"" + StringEncoderDecoder.encode(systemType) + "\"/>");
        out.println("    <param name=\"password\"          value=\"" + StringEncoderDecoder.encode(password) + "\"/>");
        out.println("    <param name=\"addrRange\"         value=\"" + StringEncoderDecoder.encode(addrRange) + "\"/>");
        out.println("    <param name=\"smeN\"              value=\"" + smeN + "\"/>");
        out.println("    <param name=\"wantAlias\"         value=\"" + (wantAlias ? "yes" : "no") + "\"/>");
        out.println("    <param name=\"forceDC\"           value=\"" + forceDC + "\"/>");
        out.println("    <param name=\"timeout\"           value=\"" + timeout + "\"/>");
        out.println("    <param name=\"receiptSchemeName\" value=\"" + StringEncoderDecoder.encode(receiptSchemeName) + "\"/>");
        out.println("    <param name=\"disabled\"          value=\"" + disabled + "\"/>");
        out.println("    <param name=\"mode\"              value=\"" + getModeStr() + "\"/>");
        if (0 < proclimit)
            out.println("    <param name=\"proclimit\"         value=\"" + proclimit + "\"/>");
        if (0 < schedlimit)
            out.println("    <param name=\"schedlimit\"        value=\"" + schedlimit + "\"/>");
        return out;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
    }


    private String getTypeStr() {
        switch (type) {
            case SMPP:
                return "smpp";
            case SS7:
                return "ss7";
            case WAP:
                return "wap";
            case MMS:
                return "mms";
            default:
                return "unknown";
        }
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
        priority = newSvc.getPriority();
        type = newSvc.getType();
        typeOfNumber = newSvc.getTypeOfNumber();
        numberingPlan = newSvc.getNumberingPlan();
        interfaceVersion = newSvc.getInterfaceVersion();
        systemType = newSvc.getSystemType();
        password = newSvc.getPassword();
        addrRange = newSvc.getAddrRange();
        smeN = newSvc.getSmeN();
        wantAlias = newSvc.isWantAlias();
        timeout = newSvc.getTimeout();
        forceDC = newSvc.isForceDC();
        receiptSchemeName = newSvc.getReceiptSchemeName();
        disabled = newSvc.isDisabled();
        mode = newSvc.getMode();
        proclimit = newSvc.getProclimit();
        schedlimit = newSvc.getSchedlimit();
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

    public int getPriority() {
        return priority;
    }

    public void setPriority(final int priority) {
        this.priority = priority;
    }

    public int getTypeOfNumber() {
        return typeOfNumber;
    }

    public void setTypeOfNumber(final int typeOfNumber) {
        this.typeOfNumber = typeOfNumber;
    }

    public int getNumberingPlan() {
        return numberingPlan;
    }

    public void setNumberingPlan(final int numberingPlan) {
        this.numberingPlan = numberingPlan;
    }

    public int getInterfaceVersion() {
        return interfaceVersion;
    }

    public void setInterfaceVersion(final int interfaceVersion) {
        this.interfaceVersion = interfaceVersion;
    }

    public String getSystemType() {
        return systemType;
    }

    public void setSystemType(final String systemType) {
        this.systemType = systemType;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getAddrRange() {
        return addrRange;
    }

    public void setAddrRange(final String addrRange) {
        this.addrRange = addrRange;
    }

    public int getSmeN() {
        return smeN;
    }

    public void setSmeN(final int smeN) {
        this.smeN = smeN;
    }

    public boolean isWantAlias() {
        return wantAlias;
    }

    public void setWantAlias(final boolean wantAlias) {
        this.wantAlias = wantAlias;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public boolean isForceDC() {
        return forceDC;
    }

    public void setForceDC(final boolean forceDC) {
        this.forceDC = forceDC;
    }

    public String getReceiptSchemeName() {
        return receiptSchemeName;
    }

    public void setReceiptSchemeName(final String receiptSchemeName) {
        this.receiptSchemeName = receiptSchemeName;
    }

    public boolean isDisabled() {
        return disabled;
    }

    public void setDisabled(final boolean disabled) {
        this.disabled = disabled;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public int getProclimit() {
        return proclimit;
    }

    public void setProclimit(final int proclimit) {
        this.proclimit = proclimit;
    }


    public int getSchedlimit() {
        return schedlimit;
    }

    public void setSchedlimit(final int schedlimit) {
        this.schedlimit = schedlimit;
    }

    public SmeStatus getStatus() {
        return status;
    }

    public void setStatus(final SmeStatus status) {
        this.status = status;
    }
}



