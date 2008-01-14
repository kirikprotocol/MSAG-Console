package ru.sibinco.smpp.ub_sme.inbalance;

import org.apache.log4j.Category;
import ru.sibinco.smpp.ub_sme.util.VarString;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 18:44:31
 * To change this template use File | Settings | File Templates.
 */
public class InBalanceResult extends InBalancePDU {
  private final static Category logger = Category.getInstance(InBalanceClient.class);

  public final static byte STATUS_OK = 0x00;
  public final static byte STAUS_ALREADY_REQUESTED = 0x01;
  public final static byte STAUS_ERROR= 0x02;

  int status = -1;

  private byte flg;
  private byte DCS = -1;
  private VarString UssData;
  private VarString msISDN;

  private boolean parsed = false;

  public final static byte CMD_ID = 0x02;

  public final static byte FLG_BINARY_DCS_ENCODED = 0x00;
  public final static byte FLG_TEXT_LATIN1 = 0x01;
  public final static byte FLG_TEXT_UCS2 = 0x02;

  public InBalanceResult() {
    super(CMD_ID);
  }

  public InBalanceResult(byte[] data) throws InBalancePDUException {
    super(data);
    if (cmdId != CMD_ID) {
      throw new InBalancePDUException("Unexpected cmdID for BalanceResult: " + cmdId);
    }
  }

  private void parsePDU() throws InBalancePDUException {
    try {
      int p = HEADER_LENGTH;
      status = ((0xFF & data[p++]) << 8) | (0xFF & data[p++]);
      if(status!=STATUS_OK){
        parsed = true;
        return;
      }
      flg = data[p++];
      String ussDataEnc = null;
      switch (flg){
        case FLG_BINARY_DCS_ENCODED:
          DCS = data[p++];
          throw new InBalancePDUException("Binary (DCS="+DCS+") encoded data is unsupported!");
        case FLG_TEXT_LATIN1:
          ussDataEnc = "ISO-8859-1";
          break;
        case FLG_TEXT_UCS2:
          ussDataEnc = "UTF-16BE";
          //ussDataEnc = "UTF-16LE";
          break;
        default:
          throw new InBalancePDUException("Unexpected \"flg\" pdu field value: "+flg);
      }
      //System.out.println("p: "+p);
      //int ussDataLen = 0xFF & data[p++];
      //p++;
      //System.out.println("ussDataLen: "+ussDataLen);
      //UssData = new String(data, p, ussDataLen, ussDataEnc);
      UssData = new VarString(data, p, ussDataEnc);
      p = p + UssData.getLength();

      //p = p + ussDataLen;
      //int msISDNLen = 0xFF & data[p++];
      //System.out.println("msISDNLen: "+msISDNLen);
      //System.out.println("p: "+p);
      //System.out.println("data.length: "+data.length);
      //msISDN = new String(data, p, msISDNLen);
      //p = p + msISDNLen;
      msISDN = new VarString(data, p);
      p = p + msISDN.getLength();

      parsed = true;
    } catch (Exception e) {
      throw new InBalancePDUException("PDU Parse error", e);
    }
  }

  public int getStatus() throws InBalancePDUException {
    if (!parsed) {
      parsePDU();
    }
    return status;
  }

  public byte getDCS() throws InBalancePDUException {
    if (!parsed) {
      parsePDU();
    }
    return DCS;
  }

  public String getUssData() throws InBalancePDUException {
    if (!parsed) {
      parsePDU();
    }
    return UssData != null ? UssData.getStringValue() : null;
  }

  public String getMsISDN() throws InBalancePDUException {
    if (!parsed) {
      parsePDU();
    }
    return msISDN != null ? msISDN.getStringValue() : null;
  }

  public byte getFlg() throws InBalancePDUException {
    if (!parsed) {
      parsePDU();
    }
    return flg;
  }

  public String toString() {
    try {
      StringBuffer sb = new StringBuffer("InBalanceResult: requestId=");
      sb.append(getRequestId());
      sb.append(", status=");
      sb.append(getStatus());
      sb.append(", flg=");
      sb.append(getFlg());
      if(getStatus() == STATUS_OK) {
        sb.append(", DCS=");
        sb.append(getDCS());
        sb.append(", ussData=");
        sb.append(getUssData());
        sb.append(", msISDN=");
        sb.append(getMsISDN());
      }
      return sb.toString();
    } catch (InBalancePDUException e) {
      return "InBalanceResult: " + e.getMessage();
    }
  }

}
