package ru.novosoft.smsc.web.applets.topmon;

import ru.novosoft.smsc.admin.topmon.SmeSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:48:00
 */
class SmeIdComparator implements Comparator<SmeSnap> {
  private final boolean asc;

  public SmeIdComparator(boolean asc) {
    this.asc = asc;
  }

  public int compare(SmeSnap snap1, SmeSnap snap2) {
    int rc = snap1.smeId.compareTo(snap2.smeId);
    return asc ? rc : -rc;
  }
}
