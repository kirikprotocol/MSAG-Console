package mobi.eyeline.informer.util;

import java.util.HashMap;
import java.util.Map;

/**
 * Дни недели
 */
public enum Day {
  Mon(1),
  Tue(2),
  Wed(3),
  Thu(4),
  Fri(5),
  Sat(6),
  Sun(7);

  private static final Map<Integer, Day> days = new HashMap<Integer, Day>(7);

  static {
    for (Day d : values()) {
      days.put(d.getDay(), d);
    }
  }

  private final int day;

  Day(int day) {
    this.day = day;
  }

  public int getDay() {
    return day;
  }

  public static Day valueOf(int d) {
    return days.get(d);
  }
}
