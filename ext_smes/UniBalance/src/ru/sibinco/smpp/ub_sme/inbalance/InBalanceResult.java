package ru.sibinco.smpp.ub_sme.inbalance;

import org.apache.log4j.Category;

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
    public final static byte STAUS_ERROR = 0x02;

    int status = -1;

    private byte flg;
    private byte DCS = -1;
    private String UssData;
    private String msISDN;

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
            if (status != STATUS_OK) {
                return;
            }
            flg = data[p++];
            String ussDataEnc = null;
            switch (flg) {
                case FLG_BINARY_DCS_ENCODED:
                    DCS = data[p++];
                    throw new InBalancePDUException("Binary (DCS=" + DCS + ") encoded data is unsupported!");
                case FLG_TEXT_LATIN1:
                    ussDataEnc = "ISO-8859-1";
                    break;
                case FLG_TEXT_UCS2:
                    ussDataEnc = "UTF-16BE";
                    break;
                default:
                    throw new InBalancePDUException("Unexpected \"flg\" pdu field value: " + flg);
            }
            int ussDataLen = 0xFF & data[p++];
            UssData = new String(data, p, ussDataLen, ussDataEnc);
            p = p + ussDataLen;
            int msISDNLen = 0xFF & data[p++];
            msISDN = new String(data, p, msISDNLen);
            p = p + msISDNLen;
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
        return UssData;
    }

    public String getMsISDN() throws InBalancePDUException {
        if (!parsed) {
            parsePDU();
        }
        return msISDN;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer("AbonentContractResult: requestId=");
        sb.append(requestId);
        sb.append(", status=");
        sb.append(status);
        if (status == STATUS_OK) {
            sb.append(", DCS=");
            sb.append(DCS);
            sb.append(", ussData=");
            sb.append(UssData);
            sb.append(", msISDN=");
            sb.append(msISDN);
        }
        return sb.toString();
    }

}
