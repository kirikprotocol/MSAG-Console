package ru.novosoft.smsc.web.applets.topmon;

import ru.novosoft.smsc.admin.topmon.SmeSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:50:59
 */
class SmeAverageSpeedComparator implements Comparator<SmeSnap> {
  private final int idx;
  private final boolean asc;

  public SmeAverageSpeedComparator(boolean asc, int idx) {
    this.idx = idx;
    this.asc = asc;
  }

  public int compare(SmeSnap snap1, SmeSnap snap2) {
    if (snap1.avgSpeed[idx] == snap2.avgSpeed[idx])
      return 0;
    else if (snap1.avgSpeed[idx] < snap2.avgSpeed[idx])
      return asc ? -1 : 1;
    else
      return asc ? 1 : -1;
  }
}

