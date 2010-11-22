package mobi.eyeline.informer.util;

import java.util.Calendar;
import java.util.Date;
import java.util.StringTokenizer;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 13:18:04
 */
public class Time {
  int hour;
  int min;
  int sec;

  public Time(String s) {
    initFromString(s);
  }

  public Time(Date d) {
    Calendar c = Calendar.getInstance();
    c.setTime(d);
    hour = c.get(Calendar.HOUR_OF_DAY);
    min = c.get(Calendar.MINUTE);
    sec = c.get(Calendar.SECOND);
  }

  private void initFromString(String s) {
    StringTokenizer st = new StringTokenizer(s, ":");
    if (st.countTokens() < 2) throw new IllegalArgumentException("Invalid time string:'" + s + "'");
    hour = Integer.parseInt(st.nextToken(), 10);
    min = Integer.parseInt(st.nextToken(), 10);
    if (st.hasMoreTokens()) {
      sec = Integer.parseInt(st.nextToken(), 10);
    } else {
      sec = 0;
    }
    if (!validate()) {
      throw new IllegalArgumentException("Invalid time string:'" + s + "'");
    }
  }

  private boolean validate() {
    return hour >= 0 && hour < 24
        && min >= 0 && min < 60
        && sec >= 0 && sec < 60;
  }

  public Time(int hour, int min, int sec) {
    this.hour = hour;
    this.min = min;
    this.sec = sec;
  }

  public Time(Time t) {
    this.hour = t.hour;
    this.min = t.min;
    this.sec = t.sec;
  }

  public int getHour() {
    return hour;
  }


  public int getMin() {
    return min;
  }


  public int getSec() {
    return sec;
  }

  public void setTimeString(String s) {
    initFromString(s);
  }

  public String getTimeString() {
    StringBuilder sb = new StringBuilder(8);
    if (hour < 10) sb.append('0');
    sb.append(hour).append(':');
    if (min < 10) sb.append('0');
    sb.append(min).append(':');
    if (sec < 10) sb.append('0');
    sb.append(sec);
    return sb.toString();
  }

  public Date getTimeDate() {
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR_OF_DAY, hour);
    c.set(Calendar.MINUTE, min);
    c.set(Calendar.SECOND, sec);
    return c.getTime();
  }

  @Override
  public String toString() {
    return getTimeString();
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Time time = (Time) o;

    if (hour != time.hour) return false;
    if (min != time.min) return false;
    if (sec != time.sec) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = hour;
    result = 31 * result + min;
    result = 31 * result + sec;
    return result;
  }
}
