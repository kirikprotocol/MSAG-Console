package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.ErrorSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:41:43
 */
public class ErrorCodeComparator implements Comparator
{
  boolean asc;

  public ErrorCodeComparator(boolean asc)
  {
    this.asc = asc;
  }

  public int compare(Object o1, Object o2)
  {
    ErrorSnap snap1 = (ErrorSnap) o1;
    ErrorSnap snap2 = (ErrorSnap) o2;
    if (snap1.code == snap2.code)
      return 0;
    else if (snap1.code < snap2.code)
      return asc ? -1 : 1;
    else
      return asc ? 1 : -1;
  }
}
