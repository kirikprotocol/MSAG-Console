package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

/**
 * User: artem
 * Date: 23.04.2008
 */

public class RWUtils {
  public static void writeInt(int value, byte[] bytes, int offset) {
    bytes[offset]     = (byte) ((value >> 24) & 0xFF);
    bytes[offset + 1] = (byte) ((value >> 16) & 0xFF);
    bytes[offset + 2] = (byte) ((value >> 8) & 0xFF);
    bytes[offset + 3] = (byte) ((value) & 0xFF);
  }

  public static int readInt(byte[] bytes, int offset) {
    int ch1 = bytes[offset];
    int ch2 = bytes[offset+1];
    int ch3 = bytes[offset+2];
    int ch4 = bytes[offset+3];
    return (((ch1 << 24) & 0xFF000000) | ((ch2 << 16) & 0x00FF0000) | ((ch3 << 8) & 0x0000FF00) | (ch4 & 0x000000FF));
  }

  public static void writeLong(long value, byte[] bytes, int offset) {
    bytes[offset]     = (byte) ((value >> 56) & 0xFF);
    bytes[offset + 1] = (byte) ((value >> 48) & 0xFF);
    bytes[offset + 2] = (byte) ((value >> 40) & 0xFF);
    bytes[offset + 3] = (byte) ((value >> 32) & 0xFF);
    bytes[offset + 4] = (byte) ((value >> 24) & 0xFF);
    bytes[offset + 5] = (byte) ((value >> 16) & 0xFF);
    bytes[offset + 6] = (byte) ((value >> 8) & 0xFF);
    bytes[offset + 7] = (byte) ((value) & 0xFF);
  }

  public static long readLong(byte[] bytes, int offset) {
    long ch1 = bytes[offset];
    long ch2 = bytes[offset+1];
    long ch3 = bytes[offset+2];
    long ch4 = bytes[offset+3];
    long ch5 = bytes[offset+4];
    long ch6 = bytes[offset+5];
    long ch7 = bytes[offset+6];
    long ch8 = bytes[offset+7];
    return (
            ((ch1 << 56) & 0xFF00000000000000L) |
            ((ch2 << 48) & 0x00FF000000000000L) |
            ((ch3 << 40) & 0x0000FF0000000000L) |
            ((ch4 << 32) & 0x000000FF00000000L) |
            ((ch5 << 24) & 0x00000000FF000000L) |
            ((ch6 << 16) & 0x0000000000FF0000L) |
            ((ch7 << 8)  & 0x000000000000FF00L) |
            (ch8         & 0x00000000000000FFL)
           );
  }

  public static void writeString(String s, byte[] bytes, int offset, int len) {
    int i;
    for (i=0; i<s.length(); i++)
      bytes[offset + i] = (byte)s.charAt(i);
    for (int j=i; j<len; j++)
      bytes[offset+j]=(byte)' ';
  }

  public static String readString(byte[] bytes, int offset, int len) {
    StringBuilder sb = new StringBuilder(len);
    char ch;
    for (int i=offset; i<offset + len; i++) {
      ch = (char)bytes[i];
      if (ch != ' ')
        sb.append(ch);
      else
        break;
    }
    return sb.toString();
  }
}
