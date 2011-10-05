package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class HourCountersSet extends CountersSet{

  private int hour = 0;

  public HourCountersSet(int hour) {
    this.hour = hour;
  }

  public HourCountersSet(long accepted, long  rejected, long delivered, long failed,
                         long rescheduled, long temporal, long i, long o, int hour) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.hour = hour;
  }

  public int getHour() {
    return hour;
  }
}
