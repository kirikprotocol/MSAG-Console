package ru.novosoft.smsc.web.applets.topmon;

import ru.novosoft.smsc.admin.topmon.ErrorSnap;

import java.util.Comparator;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 19:41:43
 */
class ErrorCodeComparator implements Comparator<ErrorSnap> {
  private final boolean asc;

  public ErrorCodeComparator(boolean asc) {
    this.asc = asc;
  }

  public int compare(ErrorSnap snap1, ErrorSnap snap2) {
    if (snap1.code == snap2.code)
      return 0;
    else if (snap1.code < snap2.code)
      return asc ? -1 : 1;
    else
      return asc ? 1 : -1;
  }
}
