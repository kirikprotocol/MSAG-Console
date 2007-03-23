package ru.sibinco.smpp.ub_sme.inbalance;

import org.apache.log4j.Category;

import java.io.InputStream;
import java.io.EOFException;
import java.io.IOException;

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

  private byte DCS;
  private String UssData;
  private String UsISDN;
  private byte IN_SSN;
  private String IN_ISDN;

  public final static byte CMD_ID = 0x02;

  public InBalanceResult() {
    super(CMD_ID);
  }

  public InBalanceResult(InputStream is) throws InBalancePDUException, IOException {
    super(is);
    if (cmdId != CMD_ID) {
      throw new InBalancePDUException("Unexpected cmdID for BalanceResult: " + cmdId);
    }
    try {
      status = (is.read() << 8) | is.read();
      if(status!=STATUS_OK){
        return;
      }
      DCS = (byte)is.read();
      int ussDataLen = is.read();
      byte[] buf;
      int read;
      buf = new byte[ussDataLen];
      read = is.read(buf);
      if(read==-1){
        throw new EOFException();
      }
      UssData = new String(buf);
      int msISDNLen = is.read();
      buf = new byte[msISDNLen];
      read = is.read(buf);
      if(read==-1){
        throw new EOFException();
      }
      UsISDN = new String(buf);
      IN_SSN = (byte)is.read();
      int IN_ISDN_Len = is.read();
      buf = new byte[IN_ISDN_Len];
      read = is.read(buf);
      if(read==-1){
        throw new EOFException();
      }
      IN_ISDN = new String(buf);
    } catch (IOException e) {
      throw e;
    } catch (Exception e) {
      throw new InBalancePDUException("PDU Parse error", e);
    }
  }

  public int getStatus() throws InBalancePDUException {
    return status;
  }

  public byte getDCS() throws InBalancePDUException {
    return DCS;
  }

  public String getUssData() throws InBalancePDUException {
    return UssData;
  }

  public String getUsISDN() throws InBalancePDUException {
    return UsISDN;
  }

  public byte getIN_SSN() throws InBalancePDUException {
    return IN_SSN;
  }

  public String getIN_ISDN() throws InBalancePDUException {
    return IN_ISDN;
  }

  public String toString() {
    StringBuffer sb = new StringBuffer("AbonentContractResult: requestId=");
    sb.append(requestId);
    sb.append(", status=");
    sb.append(status);
    if(status == STATUS_OK) {
      sb.append(", DCS=");
      sb.append(DCS);
      sb.append(", ussData=");
      sb.append(UssData);
      sb.append(", msISDN=");
      sb.append(UsISDN);
      sb.append(", IN_SSN=");
      sb.append(IN_SSN);
      sb.append(", IN_ISDN=");
      sb.append(IN_ISDN);
    }
    return sb.toString();
  }

}
