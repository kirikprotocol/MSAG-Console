package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.SmeSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:48:00
 */
public class SmeIdComparator implements Comparator {
  boolean asc;
  public SmeIdComparator(boolean asc) {
    this.asc = asc;
  }
  public int compare(Object o1, Object o2) {
    SmeSnap snap1 = (SmeSnap)o1;
    SmeSnap snap2 = (SmeSnap)o2;
    int rc = snap1.smeId.compareTo(snap2.smeId);
    return asc?rc:-rc;
  }
}
