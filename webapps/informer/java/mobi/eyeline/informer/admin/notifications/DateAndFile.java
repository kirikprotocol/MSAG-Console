package mobi.eyeline.informer.admin.notifications;

import java.io.File;
import java.util.Calendar;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 24.11.2010
 * Time: 15:27:29
 */
public class DateAndFile {
  private Calendar calendar;
  private File file;

  public DateAndFile(Calendar calendar, File f) {
    this.calendar = calendar;
    this.file = f;
  }

  public Calendar getCalendar() {
    return calendar;
  }

  public void setCalendar(Calendar c) {
    this.calendar = calendar;
  }

  public File getFile() {
    return file;
  }

  public void setFile(File file) {
    this.file = file;
  }
}
