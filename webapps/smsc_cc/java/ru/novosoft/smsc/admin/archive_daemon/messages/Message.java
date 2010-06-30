package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public abstract class Message {
  public final static byte SMSC_BYTE_UNDEF_TYPE = 0;
  public final static byte SMSC_BYTE_QUERY_TYPE = 10; // Can send only
  public final static byte SMSC_BYTE_COUNT_TYPE = 20; // Can send only
  public final static byte SMSC_BYTE_TOTAL_TYPE = 30; // Can receive only
  public final static byte SMSC_BYTE_RSSMS_TYPE = 40; // Can receive only
  public final static byte SMSC_BYTE_RSNXT_TYPE = 50; // Can send & receive
  public final static byte SMSC_BYTE_EMPTY_TYPE = 60; // Can send & receive
  public final static byte SMSC_BYTE_ERROR_TYPE = 70; // Can send & receive

  protected byte type = SMSC_BYTE_UNDEF_TYPE;

  public static Message create(byte type) {
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

  static {
    dateFormat = new SimpleDateFormat(DATE_FORMAT);
    dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
  }

  public static Date convertDateFromString(String dateString) throws IOException {
    Date date = null;
    try {
      if (dateString != null && dateString.length() > 0) date = dateFormat.parse(dateString);
    } catch (ParseException e) {
      throw new IOException("Invalid date/time format");
    }
    return date;
  }

  public static String convertStringToDate(Date date) {
    String dateString = "";
    dateString = dateFormat.format(date);
    return dateString;
  }


  protected Message(byte type) {
    this.type = type;
  }

  public byte getType() {
    return type;
  }

  public void send(OutputStream os) throws IOException {
    IOUtils.writeUInt8(os, type);
  }

  public void receive(InputStream is) throws IOException {
    // do nothing, implemented if needed in subclasses
  }
}