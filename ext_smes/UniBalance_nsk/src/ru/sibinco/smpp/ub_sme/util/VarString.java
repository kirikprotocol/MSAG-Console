package ru.sibinco.smpp.ub_sme.util;

import java.io.UnsupportedEncodingException;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 06.12.2006
 * Time: 19:21:14
 * To change this template use File | Settings | File Templates.
 */
public class VarString {
  int bodyLength = 0;
  String value;
  byte[] header;

  /**
   * Warning: this methood support only ISO-8859-1 strings
   * @param stringValue value
   */
  public VarString(String stringValue) {
    value = stringValue;
    bodyLength = (value != null ? value.length() : 0);
    int headLength = 1;
    while (((bodyLength >> (7 * headLength)) & 0xFF) != 0 && headLength < 6) {
      headLength++;
    }
    header = new byte[headLength];

    for (int i = 0; i < headLength; i++) {
      header[i] = (byte) (((bodyLength >> i * 7) & 0x7F) | ((i != headLength - 1) ? 0x80 : 0x00));
    }
  }

  public VarString(byte[] data, int offset, String enc) {
    this.header = data;
    int headerLength=0;
    for (int i = offset; i < data.length; i++) {
      byte b = data[i];
      bodyLength = bodyLength | ((0x7F & b)  << 7*(i-offset));
      headerLength++;
      if((b & 0x80) == 0){
        break;
      }
    }
    header=new byte[headerLength];
    System.arraycopy(data, offset, header, 0, headerLength);
    if(bodyLength==0){
      value = null;
      return;
    }
    if(bodyLength==1 && data[offset]==0){
      value = null;
      return;
    }
    try {
      value=new String(data, offset+header.length, bodyLength, enc);
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support "+enc, e);
    }
  }

  public VarString(byte[] data, int offset) {
    this(data, offset, "ISO-8859-1");
  }

  protected static int readVarStringLength(byte[] data, int offset){
    int result = 0;
    for (int i = offset; i < data.length; i++) {
      byte b = data[offset+i];
      result = result & (b << 7*(i-offset));      
      if((b & 0x80) == 0){
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
    if(value==null){
      return;
    }
    try {
      body=value.getBytes("ISO-8859-1");
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException("System is doesn't support ISO-8859-1 :)", e);
    }
    System.arraycopy(body, 0, data, offset+header.length, body.length);
  }

  public int getLength() {
    return header.length + (value != null ? bodyLength : 0);
  }

  public String toString() {
    return value;
  }

}
