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

    VarString nmPolicy = null;
    byte contractType = -1;
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

    public VarString getAbImsi() throws InManPDUException {
        if (!parsed) {
            parsePDU();
        }
        return abImsi;
    }

    public int getError() throws InManPDUException {
        if (!parsed) {
            parsePDU();
        }
        return error;
    }

    public VarString getErrorMsg() {
        return errorMsg;
    }

    public void setErrorMsg(VarString errorMsg) {
        this.errorMsg = errorMsg;
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
        sb.append(", abIMSI=");
        sb.append(abImsi);
        sb.append(", errorMsg=");
        sb.append(errorMsg);
        return sb.toString();
    }

}
