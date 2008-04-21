package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * User: artem
 * Date: 25.03.2008
 */

class IOUtils {

  public static void writeInt(int value, OutputStream writer) throws IOException {
    writer.write(((value >> 24) & 0xFF));
    writer.write((value >> 16) & 0xFF);
    writer.write((value >> 8) & 0xFF);
    writer.write((value) & 0xFF);
  }

  public static int readInt(InputStream is) throws IOException {
    int ch1 = readByte(is);
    int ch2 = readByte(is);
    int ch3 = readByte(is);
    int ch4 = readByte(is);
    return (((ch1 << 24) & 0xFF000000) | ((ch2 << 16) & 0x00FF0000) | ((ch3 << 8) & 0x0000FF00) | (ch4 & 0x000000FF));
  }

  public static int readByte(InputStream is) throws IOException {
    int b = is.read();
    if (b == -1)
      throw new EOFException();
    return b;
  }

  public static void writeString(String string, OutputStream writer) throws IOException {
    writer.write(string.length());
    for (int i=0; i<string.length(); i++)
      writer.write((int)string.charAt(i));
  }

  public static String readString(InputStream r) throws IOException {
    int length = r.read();
    if (length < 0)
      throw new EOFException();

    StringBuilder sb = new StringBuilder(length);
    int b;
    for (int i=0; i<length; i++) {
      if ((b = r.read()) < 0)
        throw new IOException("Broken file");
      sb.append((char)b);
    }
    return sb.toString();
  }

  public static void skipString(InputStream r) throws IOException {
    int length = r.read();
    if (length == -1)
      throw new EOFException();

    for (int skipped =0; skipped < length; skipped++) {
      if (r.read() == -1)
        throw new IOException("Broken file: " + skipped + ' ' + length);
    }
  }

}
