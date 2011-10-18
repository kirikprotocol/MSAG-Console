package ru.novosoft.smsc.util.applet;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 12:42:05
 */
public class SnapBufferReader
{
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());
  byte buf[] = new byte[1024];
  int offset;
  int size;

  public SnapBufferReader()
  {
    offset = 0;
  }

  public void resetOffset()
  {
    offset = 0;
  }

  public int read() throws EOFException
  {
    if (offset >= size) {
      logger.error("Buffer: Requested byte at " + offset + " of " + size);
      throw new EOFException();
    }
    return ((int) buf[offset++]) & 0xFF;
  }

  public void fill(InputStream is, int len) throws IOException
  {
//      logger.debug("filling buffer with: "+len+" was used: "+offset);
    if (buf.length < len) buf = new byte[len];
    offset = 0;
    size = 0;
    int cnt = 0;
    while (size < len) {
      cnt = is.read(buf, size, len - size);
      if (cnt == -1) throw new EOFException();
      size += cnt;
    }
//      logger.debug("filled buffer with: "+size);
  }

  public short readNetworkShort() throws IOException
  {
    int ch1 = read();
    int ch2 = read();
    return (short) ((ch1 << 8) + ch2);
  }

  public int readNetworkInt() throws IOException
  {
    int ch1 = read();
    int ch2 = read();
    int ch3 = read();
    int ch4 = read();
    return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4);
  }

  public int readNetworkInt(InputStream in) throws IOException
  {
    int ch1 = in.read();
    int ch2 = in.read();
    int ch3 = in.read();
    int ch4 = in.read();
    if (ch1 == -1 || ch2 == -1 || ch3 == -1 || ch4 == -1)
      throw new EOFException();
    return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4);
  }

  public long readNetworkLong() throws IOException
  {
    int i1 = readNetworkInt();
    int i2 = readNetworkInt();
    return ((long) (i1) << 32) + (i2 & 0xFFFFFFFFL);
  }

  public long readNetworkLongRev() throws IOException
  {
    int i1 = readNetworkInt();
    int i2 = readNetworkInt();
    return ((long) (i2) << 32) + (i1 & 0xFFFFFFFFL);
  }

  public String readNetworkString() throws IOException
  {
    int sz = ((int) readNetworkShort()) & 0xFFFF;
    if (offset + sz >= size) {
      logger.error("Buffer: Requested string len " + sz + " at " + offset + " out of buffer size" + size);
      throw new EOFException();
    }
    String s = new String(buf, offset, sz);
    offset += sz;
    return s;
  }

  public String readNetworkString(String charset) throws IOException
  {
    int sz = ((int) readNetworkShort()) & 0xFFFF;
    if (offset + sz >= size) {
      logger.error("Buffer: Requested string len " + sz + " at " + offset + " out of buffer size" + size);
      throw new EOFException();
    }
    String s = new String(buf, offset, sz, charset);
    offset += sz;
    return s;
  }

  public String readNetworkCFixedString(int maxLen) throws IOException
  {
    int terminator = offset;
    while (buf[terminator] != 0) terminator++;
    String s = new String(buf, offset, terminator - offset);
    offset += maxLen;
    return s;
  }

  public String readNetworkCFixedString(int maxLen, String charset) throws IOException
  {
    int terminator = offset;
    while (buf[terminator] != 0) terminator++;
    String s = new String(buf, offset, terminator - offset, charset);
    offset += maxLen;
    return s;
  }
}
