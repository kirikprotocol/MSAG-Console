package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.ErrorSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:46:12
 */
public class ErrorCountComparator implements Comparator {
  boolean asc;
  public ErrorCountComparator(boolean asc) {
    this.asc = asc;
  }
  public int compare(Object o1, Object o2) {
    ErrorSnap snap1 = (ErrorSnap)o1;
    ErrorSnap snap2 = (ErrorSnap)o2;
    if( snap1.count == snap2.count ) return 0;
    else if( snap1.count < snap2.count ) return asc?-1:1;
    else return asc?1:-1;
  }
}
