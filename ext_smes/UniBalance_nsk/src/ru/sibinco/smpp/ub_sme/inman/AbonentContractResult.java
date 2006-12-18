package ru.sibinco.smpp.ub_sme.inman;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 18:44:31
 * To change this template use File | Settings | File Templates.
 */
public class AbonentContractResult extends InManPDU {
  private final static Category logger = Category.getInstance(InManClient.class);

  public final static byte CONTRACT_UNKNOWN = 0x00;
  public final static byte CONTRACT_POSTPAID = 0x01;
  public final static byte CONTRACT_PREPAID = 0x02;

  VarString nmPolicy = null;
  byte contractType = - 1;
  VarString gsmSCFAddress = null;
  int serviceKey = -1;
  int error = -1;

  private boolean parsed = false;

  public final static byte TAG = 0x07;

  public AbonentContractResult() {
    super(TAG);
  }

  public AbonentContractResult(byte[] data) throws PDUException {
    super(data);
    if (tag != TAG) {
      throw new PDUException("Unexpected tag for AbonentContractResult: " + tag);
    }
  }

  private void parsePDU() throws PDUException {
    try {
      int p = HEADER_LENGTH;
      nmPolicy = new VarString(data, p);
      p = p + nmPolicy.getLength();
      contractType = data[p++];
      gsmSCFAddress = new VarString(data, p);
      int serviceKeyOrError = ((data[p++] & 0xFF) << 24) | ((data[p++] & 0xFF) << 16) | ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
      if (gsmSCFAddress.getStringValue().length() > 0) {
        serviceKey = serviceKeyOrError;
      } else {
        error = serviceKeyOrError;
      }
    } catch (Exception e) {
      throw new PDUException("PDU Parse error", e);
    }
  }

  public String getNmPolicy() throws PDUException {
    if (!parsed) {
      parsePDU();
    }
    return nmPolicy != null ? nmPolicy.getStringValue() : null;
  }

  public byte getContractType() throws PDUException {
    if (!parsed) {
      parsePDU();
    }
    return contractType;
  }

  public String getGsmSCFAddress() throws PDUException {
    if (!parsed) {
      parsePDU();
    }
    return gsmSCFAddress != null ? gsmSCFAddress.getStringValue() : null;
  }

  public long getServiceKey() throws PDUException {
    if (!parsed) {
      parsePDU();
    }
    return serviceKey;
  }

  public int getError() throws PDUException {
    if (!parsed) {
      parsePDU();
    }
    return error;
  }

  public String toString() {
    StringBuffer sb = new StringBuffer("AbonentContractResult: sn=");
    sb.append(dialogID);
    sb.append(", nmPolicy=");
    sb.append(nmPolicy);
    sb.append(", contractType=");
    sb.append(contractType);
    sb.append(", gsmSCFAddress=");
    sb.append(gsmSCFAddress);
    sb.append(", serviceKey=");
    sb.append(serviceKey);
    sb.append(", error=");
    sb.append(error);
    return sb.toString();
  }

}
