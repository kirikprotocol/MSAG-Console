package ru.sibinco.smpp.ub_sme.inman;

import org.apache.log4j.Category;
import ru.sibinco.smpp.ub_sme.util.VarString;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 18:44:31
 * To change this template use File | Settings | File Templates.
 */
public class AbonentContractResult extends InManPDU {
  private final static Category logger = Category.getInstance(InManClient.class);

  VarString nmPolicy = null;
  byte contractType = - 1;
  VarString gsmSCFAddress = null;
  int serviceKey = -1;
  int error = -1;
  VarString abImsi = null;
  VarString errorMsg = null;

  private boolean parsed = false;

  public final static byte TAG = 0x07;

  public AbonentContractResult() {
    super(TAG);
  }

  public AbonentContractResult(byte[] data) throws InManPDUException {
    super(data);
    if (tag != TAG) {
      throw new InManPDUException("Unexpected tag for AbonentContractResult: " + tag);
    }
  }

  private void parsePDU() throws InManPDUException {
    try {
      int p = HEADER_LENGTH;
      nmPolicy = new VarString(data, p);
      p = p + nmPolicy.getLength();
      contractType = data[p++];
      gsmSCFAddress = new VarString(data, p);
      p = p + gsmSCFAddress.getLength();
      int serviceKeyOrError = ((data[p++] & 0xFF) << 24) | ((data[p++] & 0xFF) << 16) | ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
      if (gsmSCFAddress.getStringValue() != null && gsmSCFAddress.getStringValue().length() > 0) {
        serviceKey = serviceKeyOrError;
      } else {
        error = serviceKeyOrError;
      }
      abImsi = new VarString(data, p);
      p = p + abImsi.getLength();
      errorMsg = new VarString(data, p);
      p = p + errorMsg.getLength();
      parsed = true;
    } catch (Exception e) {
      throw new InManPDUException("PDU Parse error", e);
    }
  }

  public String getNmPolicy() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return nmPolicy != null ? nmPolicy.getStringValue() : null;
  }

  public byte getContractType() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return contractType;
  }

  public String getGsmSCFAddress() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return gsmSCFAddress != null ? gsmSCFAddress.getStringValue() : null;
  }

  public long getServiceKey() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return serviceKey;
  }

  public String getAbImsi() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return abImsi != null ? abImsi.getStringValue() : null;
  }

  public int getError() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return error;
  }

  public String getErrorMsg() throws InManPDUException {
    if (!parsed) {
      parsePDU();
    }
    return errorMsg != null ? errorMsg.getStringValue() : null;
  }

  public String toString() {
    try {
      StringBuffer sb = new StringBuffer("AbonentContractResult: sn=");
      sb.append(getDialogID());
      sb.append(", nmPolicy=");
      sb.append(getNmPolicy());
      sb.append(", contractType=");
      sb.append(getContractType());
      sb.append(", gsmSCFAddress=");
      sb.append(getGsmSCFAddress());
      if(serviceKey!=-1){
        sb.append(", serviceKey=");
        sb.append(getServiceKey());
      }
      sb.append(", abIMSI=");
      sb.append(getAbImsi());
      if(error!=-1){
        sb.append(", error=");
        sb.append(getError());
        sb.append(", errorMsg=");
        sb.append(getErrorMsg());        
      }
      return sb.toString();
    } catch (InManPDUException e) {
      return "AbonentContractResult: " + e.getMessage();
    }
  }

}
