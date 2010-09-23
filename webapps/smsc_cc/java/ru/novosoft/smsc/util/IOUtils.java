package ru.novosoft.smsc.util;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Утилиты ввода/вывода
 * @author Aleksandr Khalitov
 */

public class IOUtils {

  public static void writeUInt8(OutputStream os, int value) throws IOException {
    os.write(value & 0xFF);
  }

  public static void writeUInt16(OutputStream os, int value) throws IOException {
    os.write((value >> 8) & 0xFF);
    os.write(value & 0xFF);
  }

  public static void writeUInt32(OutputStream os, long value) throws IOException {
    os.write((int) ((value >> 24) & 0xFF));
    os.write((int) ((value >> 16) & 0xFF));
    os.write((int) ((value >> 8) & 0xFF));
    os.write((int) (value & 0xFF));
  }

  public static void writeInt64(OutputStream os, long value) throws IOException {
    os.write((int) ((value >> 56) & 0xFF));
    os.write((int) ((value >> 48) & 0xFF));
    os.write((int) ((value >> 40) & 0xFF));
    os.write((int) ((value >> 32) & 0xFF));
    os.write((int) ((value >> 24) & 0xFF));
    os.write((int) ((value >> 16) & 0xFF));
    os.write((int) ((value >> 8) & 0xFF));
    os.write((int) (value & 0xFF));
  }

  public static void writeString8(OutputStream os, String str) throws IOException {
    byte[] bytes = str.getBytes();
    writeUInt8(os, bytes.length);
    os.write(bytes);
  }

  public static void writeString16(OutputStream os, String str) throws IOException {
    byte[] bytes = str.getBytes();
    writeUInt16(os, bytes.length);
    os.write(bytes);
  }

  public static void writeString(OutputStream os, String str, int len) throws IOException {
    byte[] bytes = new byte[len];
    System.arraycopy(str.getBytes(), 0, bytes, 0, len);
    os.write(bytes);
  }

  public static void writeString32(OutputStream os, String str) throws IOException {
    byte[] bytes = str.getBytes();
    writeUInt32(os, bytes.length);
    os.write(bytes);
  }

  public static int readUInt8(InputStream is) throws IOException {
    int b = is.read();
    if (b == -1) throw new EOFException();
    return b;
  }

  public static int readUInt16(InputStream is) throws IOException {
    return (readUInt8(is) << 8 | readUInt8(is));
  }

  public static long readUInt32(InputStream is) throws IOException {
    return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
        ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
  }

   public static int readInt32(InputStream is) throws IOException {
    return (readUInt8(is) << 24) | ( readUInt8(is) << 16) |
        ( readUInt8(is) << 8) | ( readUInt8(is));
  }

  public static int readUInt32Int(InputStream is) throws IOException {
    return (readUInt8(is) << 24) | (readUInt8(is) << 16) |
        (readUInt8(is) << 8) | (readUInt8(is));
  }

  public static long readInt64(InputStream is) throws IOException {
    return (readUInt32(is) << 32) | readUInt32(is);
  }

  public static String readString(InputStream is, int size) throws IOException {
    if (size <= 0) return "";
    byte buff[] = new byte[size];
    int pos = 0;
    int cnt = 0;
    while (pos < size) {
      cnt = is.read(buff, pos, size - pos);
      if (cnt == -1) throw new EOFException();
      pos += cnt;
    }
    return new String(buff).trim();
  }

  public static String readString8(InputStream is) throws IOException {
    return readString(is, readUInt8(is));
  }

  public static String readString16(InputStream is) throws IOException {
    return readString(is, readUInt16(is));
  }

  public static String readString32(InputStream is) throws IOException {
    return readString(is, (int) readUInt32(is));
  }

  public static void skip(InputStream is, int bytesToSkip) throws IOException {
    long read = 0;
    while (read < bytesToSkip) {
      long result = is.skip(bytesToSkip - read);
      
      read += result;
    }
  }

  public static void readFully(InputStream is, byte[] bytes) throws IOException {
    readFully(is, bytes, bytes.length);    
  }

  public static void readFully(InputStream is, byte[] bytes, int len) throws IOException {
    int start = 0;
    while (start < len) {
      int res = is.read(bytes, start, len - start);
      if (res == -1)
        throw new EOFException();
      start += res;
    }
  }
}
