package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.SmeSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:49:11
 */
public class SmeSpeedComparator implements Comparator
{
  int idx;
  boolean asc;

  public SmeSpeedComparator(boolean asc, int idx)
  {
    this.idx = idx;
    this.asc = asc;
  }

  public int compare(Object o1, Object o2)
  {
    SmeSnap snap1 = (SmeSnap) o1;
    SmeSnap snap2 = (SmeSnap) o2;
    if (snap1.speed[idx] == snap2.speed[idx])
      return 0;
    else if (snap1.speed[idx] < snap2.speed[idx])
      return asc ? -1 : 1;
    else
      return asc ? 1 : -1;
  }
}
