package ru.sibinco.smpp.ub_sme.inman;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class InManPDU implements java.io.Serializable {

    public final static int MAX_DIALOG_ID = 0xFFFFFFFF;

    int tag = -1;
    int headerFormat = 0x01;
    int dialogID;

    byte[] data;

    protected final static int HEADER_LENGTH = 12;

    public InManPDU(int tag) {
        this.tag = tag;
    }

    public InManPDU(byte[] data) throws InManPDUException {
        if (data.length < HEADER_LENGTH) {
            throw new InManPDUException("Illegal PDU length: " + (data.length));
        }
        this.data = data;
        try {
            int p = 4;
            tag = ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
            headerFormat = ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
            dialogID = ((data[p++] & 0xFF) << 24) | ((data[p++] & 0xFF) << 16) | ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
        } catch (Exception e) {
            throw new InManPDUException("PDU Parse error", e);
        }
    }

    protected void fillData() {
        int p = 0;

        if (data == null) {
            data = new byte[HEADER_LENGTH];
        }
        int length = data.length - 4;

        data[p++] = (byte) ((0xFF000000 & length) >> 24);
        data[p++] = (byte) ((0x00FF0000 & length) >> 16);
        data[p++] = (byte) ((0x0000FF00 & length) >> 8);
        data[p++] = (byte) ((0x000000FF & length));

        data[p++] = (byte) ((0xFF00 & tag) >> 8);
        data[p++] = (byte) ((0x00FF & tag));

        data[p++] = (byte) ((0xFF00 & headerFormat) >> 8);
        data[p++] = (byte) ((0x00FF & headerFormat));

        data[p++] = (byte) ((0xFF000000 & dialogID) >> 24);
        data[p++] = (byte) ((0x00FF0000 & dialogID) >> 16);
        data[p++] = (byte) ((0x0000FF00 & dialogID) >> 8);
        data[p++] = (byte) ((0x000000FF & dialogID));

    }

    public byte[] getData() {
        fillData();
        return data;
    }

    public int getTag() {
        return tag;
    }

    public void setTag(int tag) {
        this.tag = tag;
    }

    public int getHeaderFormat() {
        return headerFormat;
    }

    public void setHeaderFormat(int headerFormat) {
        this.headerFormat = headerFormat;
    }

    public int getDialogID() {
        return dialogID;
    }

    public void setDialogID(int dialogID) {
        this.dialogID = dialogID;
    }

    public static String bytesDebugString(byte[] bytes) {
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < bytes.length; i++) {
            int b = bytes[i] & 0xFF;
            sb.append(Integer.toHexString(b));
            sb.append(" ");
        }
        return sb.toString();
    }

}
