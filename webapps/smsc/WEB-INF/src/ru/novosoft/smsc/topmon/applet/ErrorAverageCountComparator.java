package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.ErrorSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:47:23
 */
public class ErrorAverageCountComparator implements Comparator
{
  boolean asc;

  public ErrorAverageCountComparator(boolean asc)
  {
    this.asc = asc;
  }

  public int compare(Object o1, Object o2)
  {
    ErrorSnap snap1 = (ErrorSnap) o1;
    ErrorSnap snap2 = (ErrorSnap) o2;
    if (snap1.avgCount == snap2.avgCount)
      return 0;
    else if (snap1.avgCount < snap2.avgCount)
      return asc ? -1 : 1;
    else
      return asc ? 1 : -1;
  }
}
