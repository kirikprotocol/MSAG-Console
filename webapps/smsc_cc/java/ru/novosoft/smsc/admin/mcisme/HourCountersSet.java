package ru.novosoft.smsc.admin.mcisme;

/**
 * author: Aleksandr Khalitov
 */
public class HourCountersSet extends CountersSet {
  private final int hour;

  HourCountersSet(int hour) {
    this.hour = hour;
  }

  HourCountersSet(long m, long d, long f, long n, int hour) {
    super(m, d, f, n);
    this.hour = hour;
  }

  HourCountersSet(CountersSet set, int hour) {
    super(set);
    this.hour = hour;
  }

  public int getHour() {
    return hour;
  }

}
