package ru.novosoft.smsc.admin.smsview.archive;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 19:53:17
 * To change this template use Options | File Templates.
 */
public abstract class Message
{
  public final static byte SMSC_BYTE_UNDEF_TYPE = 0;
  public final static byte SMSC_BYTE_QUERY_TYPE = 10; // Can send only
  public final static byte SMSC_BYTE_COUNT_TYPE = 20; // Can send only
  public final static byte SMSC_BYTE_TOTAL_TYPE = 30; // Can receive only
  public final static byte SMSC_BYTE_RSSMS_TYPE = 40; // Can receive only
  public final static byte SMSC_BYTE_RSNXT_TYPE = 50; // Can send & receive
  public final static byte SMSC_BYTE_EMPTY_TYPE = 60; // Can send & receive
  public final static byte SMSC_BYTE_ERROR_TYPE = 70; // Can send & receive

  protected byte type = SMSC_BYTE_UNDEF_TYPE;

  public static Message create(byte type)
  {
    switch (type) {
      case SMSC_BYTE_TOTAL_TYPE:
        return new TotalMessage();
      case SMSC_BYTE_RSSMS_TYPE:
        return new RsSmsMessage();
      case SMSC_BYTE_ERROR_TYPE:
        return new ErrorMessage();
      case SMSC_BYTE_EMPTY_TYPE:
        return new EmptyMessage();
      default:
        break;
    }
    return null;
  }

  private final static String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  private final static SimpleDateFormat dateFormat;

  static
  {
    dateFormat = new SimpleDateFormat(DATE_FORMAT);
    dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
  }

  public static Date convertDateFromString(String dateString) throws IOException
  {
    Date date = null;
    try {
      if (dateString != null && dateString.length() > 0) date = dateFormat.parse(dateString);
    } catch (ParseException e) {
      throw new IOException("Invalid date/time format");
    }
    return date;
  }

  public static String convertStringToDate(Date date)
  {
    String dateString = "";
    dateString = dateFormat.format(date);
    return dateString;
  }

  public static void writeUInt8(OutputStream os, int value) throws IOException
  {
    os.write(value & 0xFF);
  }

  public static void writeUInt16(OutputStream os, int value) throws IOException
  {
    os.write((value >> 8) & 0xFF);
    os.write(value & 0xFF);
  }

  public static void writeUInt32(OutputStream os, long value) throws IOException
  {
    os.write((int) ((value >> 24) & 0xFF));
    os.write((int) ((value >> 16) & 0xFF));
    os.write((int) ((value >> 8) & 0xFF));
    os.write((int) (value & 0xFF));
  }

  public static void writeInt64(OutputStream os, long value) throws IOException
  {
    os.write((int) ((value >> 56) & 0xFF));
    os.write((int) ((value >> 48) & 0xFF));
    os.write((int) ((value >> 40) & 0xFF));
    os.write((int) ((value >> 32) & 0xFF));
    os.write((int) ((value >> 24) & 0xFF));
    os.write((int) ((value >> 16) & 0xFF));
    os.write((int) ((value >> 8) & 0xFF));
    os.write((int) (value & 0xFF));
  }

  public static void writeString8(OutputStream os, String str) throws IOException
  {
    writeUInt8(os, str.length());
    os.write(str.getBytes());
  }

  public static void writeString16(OutputStream os, String str) throws IOException
  {
    writeUInt16(os, str.length());
    os.write(str.getBytes());
  }

  public static void writeString32(OutputStream os, String str) throws IOException
  {
    writeUInt32(os, str.length());
    os.write(str.getBytes());
  }

  public static int readUInt8(InputStream is) throws IOException
  {
    int b = is.read();
    if (b == -1) throw new EOFException();
    return b;
  }

  public static int readUInt16(InputStream is) throws IOException
  {
    return (readUInt8(is) << 8 | readUInt8(is));
  }

  public static long readUInt32(InputStream is) throws IOException
  {
    return ((long) readUInt8(is) << 24) | ((long) readUInt8(is) << 16) |
            ((long) readUInt8(is) << 8) | ((long) readUInt8(is));
  }

  public static long readInt64(InputStream is) throws IOException
  {
    return (readUInt32(is) << 32) | readUInt32(is);
  }

  public static String readString(InputStream is, int size) throws IOException
  {
    if (size <= 0) return "";
    byte buff[] = new byte[size];
    int pos = 0;
    int cnt = 0;
    while (pos < size) {
      cnt = is.read(buff, pos, size - pos);
      if (cnt == -1) throw new EOFException();
      pos += cnt;
    }
    return new String(buff);
  }

  public static String readString8(InputStream is) throws IOException
  {
    return readString(is, readUInt8(is));
  }

  public static String readString16(InputStream is) throws IOException
  {
    return readString(is, readUInt16(is));
  }

  public static String readString32(InputStream is) throws IOException
  {
    return readString(is, (int) readUInt32(is));
  }

  protected Message(byte type)
  {
    this.type = type;
  }

  public byte getType()
  {
    return type;
  }

  public void send(OutputStream os) throws IOException
  {
    writeUInt8(os, type);
  }

  public void receive(InputStream is) throws IOException
  {
    // do nothing, implemented if needed in subclasses
  }
}
