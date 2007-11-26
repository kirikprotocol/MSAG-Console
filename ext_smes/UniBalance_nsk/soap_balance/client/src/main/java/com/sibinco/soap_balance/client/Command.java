package com.sibinco.soap_balance.client;

import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public abstract class Command {
  public final static int DEFAULT_RESP=0x0100;

  public final static int SEND_PING =0x0000;

  public final static int GET_BALANCE=0x0001;
  public final static int GET_BALANCE_RESP=0x0101;

  protected int commandId;
  protected PDU pdu;

  protected Command(int commandId){
    this.pdu=new PDU();
    this.commandId=commandId;
    pdu.setCommand(commandId);
  }

  public Command(PDU pdu) throws ProcessException {
    this.pdu = pdu;
    parsePDU(pdu);
  }

  public PDU getPdu() {
    return pdu;
  }

  public void setPdu(PDU pdu) throws ProcessException {
    this.pdu = pdu;
    parsePDU(pdu);
  }

  private void parsePDU(PDU pdu) throws ProcessException {
    this.commandId=pdu.getCommand();
  }

  public int getCommandId() {
    return commandId;
  }

  public void setCommandId(int commandId) {
    this.commandId = commandId;
    pdu.setCommand(commandId);
  }

  protected static short parseShort(byte[] bytes) throws ProcessException {
    if(bytes.length<1){
      return -1;
    }
    if(bytes.length>2){
      throw new ProcessException("Illegal bytes count to parse short value: "+bytes.length);
    }
    return (short)parseLong(bytes);
  }

  protected static int parseInt(byte[] bytes) throws ProcessException {
    if(bytes.length<1){
      return -1;
    }
    if(bytes.length>4){
      throw new ProcessException("Illegal bytes count to parse int value: "+bytes.length);
    }
    return (int)parseLong(bytes);
  }

  protected static long parseLong(byte[] bytes) throws ProcessException {
    if(bytes.length<1){
      return -1;
    }
    if(bytes.length>8){
      throw new ProcessException("Illegal bytes count to parse long value: "+bytes.length);
    }
    long result=0;
    for(int b = bytes.length-1; b>=0; b--) {
      result = result | ((0xFFL & bytes[b]) << ((bytes.length-1)*8-b*8));
    }
    return result;
  }

  protected static float parseFloat(byte[] bytes) throws ProcessException {
    return Float.intBitsToFloat(parseInt(bytes));
  }

  protected static byte[] toBytes(short num) {
    return toBytes(num, 2);
  }

  protected static byte[] toBytes(short num, int size) {
    return toBytes((long)num, size);
  }

  protected static byte[] toBytes(int num) {
    return toBytes(num, 4);
  }

  protected static byte[] toBytes(int num, int size){
    return toBytes((long)num, size);
  }

  protected static byte[] toBytes(long num){
    return toBytes(num, 8);
  }

  protected static byte[] toBytes(long num, int size){
    if(size>8) {
      throw new IllegalArgumentException("Illegal long size: "+size);
    }
    byte[] bytes=new byte[size];
    for (int b = bytes.length-1; b >= 0; b--) {
      bytes[b] = (byte) ((num & (0xFFL << ((bytes.length-1)*8-b*8))) >> ((bytes.length-1)*8-b*8));
    }
    return bytes;
  }

  protected static byte[] toBytes(float num){
    return toBytes(Float.floatToIntBits(num), 4);
  }

  public String toString(){
    return "command_id: "+commandId;
  }

  public static PDU defaultResponse() {
    PDU pdu=new PDU();
    pdu.setCommand(DEFAULT_RESP);
    return pdu;
  }
}
