package ru.novosoft.smsc.admin.console.commands.sme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.route.SME;
import antlr.RecognitionException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 16:48:08
 * To change this template use File | Settings | File Templates.
 */
public abstract class SmeGenCommand extends CommandClass
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
    protected String systemType = "";
    protected String password = "";
    protected int timeout = 0;
    protected String receiptScheme = "";
    protected int proclimit = 0;
    protected int schedlimit = 0;
    protected int accessMask = 1;
    protected boolean wantAlias = false;
    protected boolean disabled = false;
    protected boolean disconnect = false;
    protected boolean carryOrgDescriptor;
    protected boolean carryOrgUserInfo;
    protected boolean carrySccpInfo;
    protected boolean fillExtraDescriptor;
    protected boolean forceSmeReceipt;
    protected boolean forceGsmDataCoding;
    protected boolean smppPlus;
    protected boolean defaultDcLatin1;

    protected boolean isMode = false;
    protected boolean isType = false;
    protected boolean isSmeN = false;
    protected boolean isAddressRange = false;
    protected boolean isPriority = false;
    protected boolean isTON = false;
    protected boolean isNPI = false;
    protected boolean isSystemType = false;
    protected boolean isPassword = false;
    protected boolean isTimeout = false;
    protected boolean isReceiptScheme = false;
    protected boolean isProclimit = false;
    protected boolean isSchedlimit = false;
    protected boolean isWantAlias = false;
    protected boolean isDisabled = false;
    protected boolean isAccessMask = false;
    protected boolean isCarryOrgDescriptor;
    protected boolean isCarryOrgUserInfo;
    protected boolean isCarrySccpInfo;
    protected boolean isFillExtraDescriptor; // for Extra mode
    protected boolean isForceSmeReceipt;
    protected boolean isForceGsmDataCoding;
    protected boolean isSmppPlus;
    protected boolean isDefaultDcLatin1;

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
    public void setDisabled(boolean disabled) {
        this.disabled = disabled; isDisabled = true;
    }
    public void setDisconnect() {
        this.disconnect = true;
    }
    public void setAccessMask(int accessMask) {
        this.accessMask = accessMask; isAccessMask = true;
    }

    public void setAccessBit(boolean value, int bitNum) throws RecognitionException {
        if (bitNum < 0 && bitNum > 31) throw new RecognitionException("incorrect bit value");
        if (value) accessMask = accessMask | (1 << bitNum);
              else accessMask = accessMask & ~(1 << bitNum);
        this.isAccessMask = true;
    }

  public void setCarryOrgDescriptor(boolean carryOrgDescriptor) {
    this.carryOrgDescriptor = carryOrgDescriptor;
    isCarryOrgDescriptor = true;
  }

  public void setCarryOrgUserInfo(boolean carryOrgUserInfo) {
    this.carryOrgUserInfo = carryOrgUserInfo;
    isCarryOrgUserInfo = true;
  }

  public void setCarrySccpInfo(boolean carrySccpInfo) {
    this.carrySccpInfo = carrySccpInfo;
    isCarrySccpInfo = true;
  }

  public void setFillExtraDescriptor(boolean fillExtraDescriptor) {
    this.fillExtraDescriptor = fillExtraDescriptor;
    isFillExtraDescriptor = true;
  }

  public void setForceSmeReceipt(boolean forceSmeReceipt) {
    this.forceSmeReceipt = forceSmeReceipt;
    isForceSmeReceipt = true;
  }

  public void setForceGsmDataCoding(boolean forceGsmDataCoding) {
    this.forceGsmDataCoding = forceGsmDataCoding;
    isForceGsmDataCoding = true;
  }

  public void setSmppPlus(boolean smppPlus) {
    this.smppPlus = smppPlus;
    isSmppPlus = true;
  }

  public void setDefaultDcLatin1(boolean defaultDcLatin1) {
    this.defaultDcLatin1 = defaultDcLatin1;
    isDefaultDcLatin1 = true;
  }
}
