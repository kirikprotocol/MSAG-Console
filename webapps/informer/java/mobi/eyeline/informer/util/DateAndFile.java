package mobi.eyeline.informer.util;

import java.io.File;
import java.util.Calendar;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 24.11.2010
 * Time: 15:27:29
 */
public class DateAndFile {
  private final Calendar calendar;
  private final File file;

  public DateAndFile(Calendar calendar, File f) {
    this.calendar = calendar;
    this.file = f;
  }

  public Calendar getCalendar() {
    return calendar;
  }

  public File getFile() {
    return file;
  }

}
