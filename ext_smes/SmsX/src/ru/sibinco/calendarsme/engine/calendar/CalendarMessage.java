package ru.sibinco.calendarsme.engine.calendar;

import java.sql.Timestamp;
import java.util.Date;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

final class CalendarMessage {

  private final int id;
  private final String source;
  private final String dest;
  private final Timestamp sendDate;
  private final String message;

  public CalendarMessage(final int id, final String source, final String dest,
                         final Timestamp sendDate, final String message) {
    this.id = id;
    this.source = source;
    this.dest = dest;
    this.sendDate = sendDate;
    this.message = message;
  }

  public CalendarMessage(final String source, final String dest, final Date sendDate, final String message) {
    this(-1, source, dest, new Timestamp(sendDate.getTime()), message);
  }

  public String getSource() {
    return source;
  }

  public String getDest() {
    return dest;
  }

  public Timestamp getSendDate() {
    return sendDate;
  }

  public String getMessage() {
    return message;
  }

  public int getId() {
    return id;
  }

  public boolean isExists() {
    return id != -1;
  }
}
