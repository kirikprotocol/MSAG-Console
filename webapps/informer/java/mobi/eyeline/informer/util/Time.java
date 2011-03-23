package mobi.eyeline.informer.util;

import java.util.Calendar;
import java.util.Date;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

/**
 * Класс, описывающий время
 */
public class Time implements Comparable {

  private int hour;
  private int min;
  private int sec;

  /**
   * Инициализирует объект из строки формата HH:mm:ss или HH:mm
   *
   * @param s строка, содержащая время
   * @throws IllegalArgumentException если строка имеет некорректный формат
   * @throws NullPointerException если s == null
   */
  public Time(String s) {
    initFromString(s);
  }

  /**
   * Инициализщирует объект, взяв время из даты
   *
   * @param d дата
   * @throws NullPointerException если d == null
   */
  public Time(Date d) {
    Calendar c = Calendar.getInstance();
    c.setTime(d);
    init(c.get(Calendar.HOUR_OF_DAY), c.get(Calendar.MINUTE), c.get(Calendar.SECOND));
  }

  /**
   * Инициализирует объект
   *
   * @param hour часы
   * @param min  минуты
   * @param sec  секунды
   * @throws IllegalArgumentException если передаваемые значения некорректны
   */
  public Time(int hour, int min, int sec) {
    init(hour, min, sec);
  }

  /**
   * Инициализирует объект
   *
   * @param timeInMillis  время в секундах
   */
  public Time(long timeInMillis) {
    Calendar c = Calendar.getInstance();
    c.setTimeInMillis(timeInMillis);
    hour = c.get(Calendar.HOUR_OF_DAY);
    min = c.get(Calendar.MINUTE);
    sec = c.get(Calendar.SECOND);
  }

  private static Calendar getCalendar(Time t) {
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR_OF_DAY, t.hour);
    c.set(Calendar.MINUTE, t.min);
    c.set(Calendar.SECOND, t.sec);
    return c;

  }

  public boolean isInInterval(Time l, Time r) {
    if(l.equals(r)) {
      return true;
    }
    Calendar lCal = getCalendar(l);
    Date rD = r.getTimeDate();
    Date thisD = getTimeDate();
    Date lD;
    if(l.hour > r.hour || (l.hour ==  r.hour && l.min>r.min) || (l.hour ==  r.hour && l.min == r.min && l.sec > r.sec)) { // 21:32 - 6:54
      lCal.add(Calendar.DAY_OF_YEAR, -1);
    }
    lD = lCal.getTime();
    return lD.compareTo(thisD)<=0 && thisD.compareTo(rD) <= 0;
  }


  /**
   * Инициализирует объект, копируя значения из другого объекта типа Time
   * @param t копия
   * @throws NullPointerException если t == null
   */
  public Time(Time t) {
    init(t.hour, t.min, t.sec);
  }

  private void init(int hour, int min, int sec) {
    if (hour < 0 || min < 0 || sec < 0)
      throw new IllegalArgumentException("Can't set negative value to time argument");
    if (min >= 60)
      throw new IllegalArgumentException("Invalid minutes value");
    if (sec >= 60)
      throw new IllegalArgumentException("Invalid seconds value");

    this.hour = hour;
    this.min = min;
    this.sec = sec;
  }

  private void initFromString(String s) {
    if (s == null)
      throw new NullPointerException();
    int hour, min, sec=0;
    try {
      StringTokenizer st = new StringTokenizer(s+" ", ":");
      hour = Integer.parseInt(st.nextToken(), 10);
      min = Integer.parseInt(st.nextToken().trim(), 10);
      if (st.hasMoreTokens())
        sec = Integer.parseInt(st.nextToken().trim(), 10);
      if (st.hasMoreTokens())
        throw new IllegalArgumentException("Invalid time string:'" + s + "'");
    } catch (NumberFormatException e) {
      throw new IllegalArgumentException("Invalid time string:'" + s + "'");
    }

    init(hour,min,sec);
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

  /**
   * Возвращщает время в виде строки формата HH:mm:ss
   * @return время в виде строки формата HH:mm:ss
   */
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

  /**
   * Возвращает время в виде объекта типа Date
   * @return время в виде объекта типа Date
   */
  public Date getTimeDate() {
    return getCalendar(this).getTime();
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

  @Override
  public int compareTo(Object o) {
    Time t = (Time) o;
    int res = Integer.valueOf(hour).compareTo(t.hour);
    if (res != 0)
      return res;
    res = Integer.valueOf(min).compareTo(t.min);
    if (res != 0)
      return res;
    res = Integer.valueOf(sec).compareTo(t.sec);
    return res;
  }
}