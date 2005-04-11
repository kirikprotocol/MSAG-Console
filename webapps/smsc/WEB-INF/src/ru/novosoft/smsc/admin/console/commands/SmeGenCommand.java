package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.route.SME;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 16:48:08
 * To change this template use File | Settings | File Templates.
 */
public abstract class SmeGenCommand implements Command
{
    public static final byte TYPE_SMPP = SME.SMPP;
    public static final byte TYPE_SS7  = SME.SS7;

    public static final byte MODE_TX = SME.MODE_TX;
    public static final byte MODE_RX = SME.MODE_RX;
    public static final byte MODE_TRX = SME.MODE_TRX;

    protected String smeId = null;
    protected byte mode = MODE_TRX;
    protected byte type = TYPE_SMPP;
    protected int smeN = 0;
    protected String addressRange = "";
    protected int priority = 0;
    protected int TON = 0;
    protected int NPI = 0;
    protected int interfaceVersion = 34;
    protected String systemType = "";
    protected String password = "";
    protected int timeout = 0;
    protected String receiptScheme = "";
    protected int proclimit = 0;
    protected int schedlimit = 0;
    protected boolean wantAlias = false;
    protected boolean forceDC = false;
    protected boolean disabled = false;
    protected boolean disconnect = false;

    protected boolean isMode = false;
    protected boolean isType = false;
    protected boolean isSmeN = false;
    protected boolean isAddressRange = false;
    protected boolean isPriority = false;
    protected boolean isTON = false;
    protected boolean isNPI = false;
    protected boolean isInterfaceVersion = false;
    protected boolean isSystemType = false;
    protected boolean isPassword = false;
    protected boolean isTimeout = false;
    protected boolean isReceiptScheme = false;
    protected boolean isProclimit = false;
    protected boolean isSchedlimit = false;
    protected boolean isWantAlias = false;
    protected boolean isForceDC = false;
    protected boolean isDisabled = false;

    public void setSmeId(String smeId) {
        this.smeId = smeId;
    }
    public void setMode(byte mode) {
        this.mode = mode; isMode = true;
    }
    public void setType(byte type) {
        this.type = type; isType = true;
    }
    public void setSmeN(int smeN) {
        this.smeN = smeN; isSmeN = true;
    }
    public void setAddressRange(String addressRange) {
        this.addressRange = addressRange; isAddressRange = true;
    }
    public void setPriority(int priority) {
        this.priority = priority; isPriority = true;
    }
    public void setTON(int TON) {
        this.TON = TON; isTON = true;
    }
    public void setNPI(int NPI) {
        this.NPI = NPI; isNPI = true;
    }
    public void setInterfaceVersion(int interfaceVersion) {
        this.interfaceVersion = interfaceVersion; isInterfaceVersion = true;
    }
    public void setSystemType(String systemType) {
        this.systemType = systemType; isSystemType = true;
    }
    public void setPassword(String password) {
        this.password = password; isPassword = true;
    }
    public void setTimeout(int timeout) {
        this.timeout = timeout; isTimeout = true;
    }
    public void setReceiptScheme(String receiptScheme) {
        this.receiptScheme = receiptScheme; isReceiptScheme = true;
    }
    public void setProclimit(int proclimit) {
        this.proclimit = proclimit; isProclimit = true;
    }
    public void setSchedlimit(int schedlimit) {
        this.schedlimit = schedlimit; isSchedlimit = true;
    }
    public void setWantAlias(boolean wantAlias) {
        this.wantAlias = wantAlias; isWantAlias = true;
    }
    public void setForceDC(boolean forceDC) {
        this.forceDC = forceDC; isForceDC = true;
    }
    public void setDisabled(boolean disabled) {
        this.disabled = disabled; isDisabled = true;
    }
    public void setDisconnect() {
        this.disconnect = true;
    }
}
