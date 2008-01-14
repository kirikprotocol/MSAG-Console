package ru.sibinco.smpp.ub_sme.inbalance;

import ru.sibinco.smpp.ub_sme.util.VarString;

import java.io.UnsupportedEncodingException;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 18:44:31
 * To change this template use File | Settings | File Templates.
 */
public class InBalanceRequest extends InBalancePDU {

  private VarString ussData;
  private String msISDN;
  private byte IN_SSN;
  private String IN_ISDN;
  private String IMSI;

  private final static byte flg = 1;

  public final static byte CMD_ID = 0x01;

  public InBalanceRequest() {
    super(CMD_ID);
  }

  protected void fillData() throws InBalancePDUException {
    if (ussData == null) {
      throw new InBalancePDUException("PDU field \"ussData\" is null");
    }
    int ussDataLen = ussData.getStringValue().length();
    if (ussDataLen < 1 || ussDataLen > 160) {
      throw new InBalancePDUException("PDU field \"ussData\" length invalid: " + ussDataLen);
    }
    if (msISDN == null) {
      throw new InBalancePDUException("PDU field \"msISDN\" is null");
    }
    int msISDNLen = msISDN.length();
    if (msISDNLen < 1 || msISDNLen > 32) {
      throw new InBalancePDUException("PDU field \"msISDN\" length invalid: " + msISDNLen);
    }
    if (IN_ISDN == null) {
      throw new InBalancePDUException("PDU field \"IN_ISDN\" is null");
    }
    int IN_ISDN_Len = IN_ISDN.length();
    if (IN_ISDN_Len < 1 || IN_ISDN_Len > 32) {
      throw new InBalancePDUException("PDU field \"IN_ISDN_Len\" length invalid: " + IN_ISDN_Len);
    }
    int IMSI_Len = IMSI!=null ? IMSI.length() : 0;
    if (IMSI_Len > 32) {
      throw new InBalancePDUException("PDU field \"IMSI_Len\" length invalid: " + IMSI_Len);
    }
    int length = HEADER_LENGTH + 6 + ussDataLen + msISDNLen + IN_ISDN_Len + IMSI_Len;
    data = new byte[length];
    super.fillData();
    int p = HEADER_LENGTH;
    data[p++] = flg;
    /*
    data[p++] = (byte) (0xFF & ussDataLen);
    try {
      byte[] buf = ussData.getBytes("ISO-8859-1");
      System.arraycopy(buf, 0, data, p, ussDataLen);
      p = p + ussDataLen;
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
    }
    */
    ussData.toBytesArray(data, p);
    p=p+ussData.getLength();

    data[p++] = (byte) (0xFF & msISDNLen);
    try {
      byte[] buf = msISDN.getBytes("ISO-8859-1");
      System.arraycopy(buf, 0, data, p, msISDNLen);
      p = p + msISDNLen;
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
    }
    data[p++] = IN_SSN;
    data[p++] = (byte) (0xFF & IN_ISDN_Len);
    try {
      byte[] buf = IN_ISDN.getBytes("ISO-8859-1");
      System.arraycopy(buf, 0, data, p, IN_ISDN_Len);
      p = p + IN_ISDN_Len;
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
    }
    data[p++] = (byte) (0xFF & IMSI_Len);
    if(IMSI_Len>0)
    try {
      byte[] buf = IMSI.getBytes("ISO-8859-1");
      System.arraycopy(buf, 0, data, p, IMSI_Len);
      p = p + IMSI_Len;
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
    }
  }

  public String getUssData() {
    return ussData!=null ? ussData.getStringValue(): null;
  }

  public void setUssData(String ussData) {
    this.ussData = new VarString(ussData);
  }

  public String getMsISDN() {
    return msISDN;
  }

  public void setMsISDN(String msISDN) {
    this.msISDN = msISDN;
  }

  public byte getIN_SSN() {
    return IN_SSN;
  }

  public void setIN_SSN(byte IN_SSN) {
    this.IN_SSN = IN_SSN;
  }

  public String getIN_ISDN() {
    return IN_ISDN;
  }

  public void setIN_ISDN(String IN_ISDN) {
    this.IN_ISDN = IN_ISDN;
  }


  public String getIMSI() {
    return IMSI;
  }

  public void setIMSI(String IMSI) {
    this.IMSI = IMSI;
  }

  public String toString() {
    StringBuffer sb = new StringBuffer("BalanceRequest: requestId=");
    sb.append(requestId);
    sb.append(", ussData=");
    sb.append(ussData);
    sb.append(", msISDN=");
    sb.append(msISDN);
    sb.append(", IN_SSN=");
    sb.append(IN_SSN);
    sb.append(", IN_ISDN=");
    sb.append(IN_ISDN);
    sb.append(", IMSI=");
    sb.append(IMSI);
    return sb.toString();
  }

}
