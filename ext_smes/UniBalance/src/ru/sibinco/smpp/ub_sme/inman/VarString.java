package ru.sibinco.smpp.ub_sme.inman;

import java.io.UnsupportedEncodingException;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 19:21:14
 * To change this template use File | Settings | File Templates.
 */
public class VarString {
    String value;
    byte[] header;

    public VarString(String stringValue) {
        value = stringValue;
        int len = (value != null ? value.length() : 0);
        int headLen = 1;
        while (((len >> (7 * headLen)) & 0xFF) != 0 && headLen < 6) {
            headLen++;
        }
        header = new byte[headLen];
        for (int i = 0; i < headLen; i++) {
            header[i] = (byte) (((len >> i * 7) & 0x7F) | ((i != headLen - 1) ? 0x80 : 0x00));
        }
    }

    public VarString(byte[] data, int offset) {
        this.header = data;
        int bodyLength = 0;
        int headerLength = 0;
        for (int i = offset; i < data.length; i++) {
            byte b = data[i];
            bodyLength = bodyLength | (b << 7 * (i - offset));
            headerLength++;
            if ((b & 0x80) == 0) {
                break;
            }
        }
        header = new byte[headerLength];
        System.arraycopy(data, offset, header, 0, headerLength);
        if (bodyLength == 0) {
            value = null;
            return;
        }
        if (bodyLength == 1 && data[offset] == 0) {
            value = null;
            return;
        }
        try {
            value = new String(data, offset + header.length, bodyLength, "ISO-8859-1");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
        }
    }

    protected static int readVarStringLength(byte[] data, int offset) {
        int result = 0;
        for (int i = offset; i < data.length; i++) {
            byte b = data[offset + i];
            result = result & (b << 7 * (i - offset));
            if ((b & 0x80) == 0) {
                break;
            }
        }
        return result;
    }

    public String getStringValue() {
        return value;
    }

    public void toBytesArray(byte[] data, int offset) {
        System.arraycopy(header, 0, data, offset, header.length);
        byte[] body;
        if (value == null) {
            return;
        }
        try {
            body = value.getBytes("ISO-8859-1");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
        }
        System.arraycopy(body, 0, data, offset + header.length, body.length);
    }

    public int getLength() {
        return header.length + (value != null ? value.length() : 0);
    }

    public String toString() {
        return value;
    }

}
