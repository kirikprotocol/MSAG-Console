package mobi.eyeline.smsquiz.distribution;

import java.util.Calendar;
import java.util.Date;
import java.util.EnumSet;
import java.util.Iterator;

public class Distribution {
  public String getTaskName() {
    return taskName;
  }

  public void setTaskName(String taskName) {
    this.taskName = taskName;
  }

  public static enum WeekDays {
    Mon, Tue, Wed, Thu, Fri, Sat, Sun
  }

  private String sourceAddress;

  private String filePath;

  private Date dateBegin;

  private Date dateEnd;

  private Calendar timeBegin;

  private Calendar timeEnd;

  private final EnumSet<WeekDays> days;

  private boolean txmode;

  private String taskName;

  public Distribution() {
    days = EnumSet.noneOf(WeekDays.class);
  }

  public String getFilePath() {
    return filePath;
  }

  public void setFilePath(String filePath) {
    this.filePath = filePath;
  }

  public Date getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(Date dateEnd) {
    this.dateEnd = dateEnd;
  }

  public Calendar getTimeBegin() {
    return timeBegin;
  }

  public void setTimeBegin(Calendar timeBegin) {
    this.timeBegin = timeBegin;
  }

  public Calendar getTimeEnd() {
    return timeEnd;
  }

  public void setTimeEnd(Calendar timeEnd) {
    this.timeEnd = timeEnd;
  }

  public void addDay(WeekDays weekDays) {
    days.add(weekDays);
  }

  public EnumSet<WeekDays> getDays() {
    return EnumSet.copyOf(days);
  }

  public boolean isTxmode() {
    return txmode;
  }

  public void setTxmode(boolean txmode) {
    this.txmode = txmode;
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getDays(String delim) {
    StringBuilder builder = new StringBuilder();
    if (days != null) {
      Iterator<WeekDays> iter = days.iterator();
      if (iter.hasNext()) {
        builder.append(iter.next().toString());
      }
      while (iter.hasNext()) {
        builder.append(delim).append(iter.next().toString());
      }
    }
    return builder.toString();
  }
}
 
