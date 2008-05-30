package ru.sibinco.smpp.ub_sme.inbalance;

import java.io.InputStream;
import java.io.IOException;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class InBalancePDU implements java.io.Serializable {

  public final static int MAX_DIALOG_ID = 0xFFFFFFFF;

  int cmdId = -1;
  int requestId;

  byte[] data;

  protected final static int HEADER_LENGTH = 10;

  public InBalancePDU(int cmdId) {
    this.cmdId = cmdId;
  }

  public InBalancePDU(int cmdId, int requestId) {
    this.cmdId = cmdId;
    this.requestId = requestId;
  }

  public InBalancePDU(byte[] data) throws InBalancePDUException {
    if (data.length < HEADER_LENGTH) {
      throw new InBalancePDUException("Illegal PDU length: " + (data.length));
    }
    this.data = data;
    try {
      int p = 4;
      cmdId = ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
      requestId = ((data[p++] & 0xFF) << 24) | ((data[p++] & 0xFF) << 16) | ((data[p++] & 0xFF) << 8) | (data[p++] & 0xFF);
    } catch (Exception e) {
      throw new InBalancePDUException("PDU read error: " + e, e);
    }
  }

  protected void fillData() throws InBalancePDUException {
    int p = 0;

    if (data == null) {
      data = new byte[HEADER_LENGTH];
    }
    int length = data.length - 4;

    data[p++] = (byte) ((0xFF000000 & length) >> 24);
    data[p++] = (byte) ((0x00FF0000 & length) >> 16);
    data[p++] = (byte) ((0x0000FF00 & length) >> 8);
    data[p++] = (byte) ((0x000000FF & length));

    data[p++] = (byte) ((0xFF00 & cmdId) >> 8);
    data[p++] = (byte) ((0x00FF & cmdId));

    data[p++] = (byte) ((0xFF000000 & requestId) >> 24);
    data[p++] = (byte) ((0x00FF0000 & requestId) >> 16);
    data[p++] = (byte) ((0x0000FF00 & requestId) >> 8);
    data[p++] = (byte) ((0x000000FF & requestId));

  }

  public byte[] getData() throws InBalancePDUException {
    fillData();
    return data;
  }

  public int getCmdId() {
    return cmdId;
  }

  public void setCmdId(int cmdId) {
    this.cmdId = cmdId;
  }

  public int getRequestId() {
    return requestId;
  }

  public void setRequestId(int requestId) {
    this.requestId = requestId;
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
