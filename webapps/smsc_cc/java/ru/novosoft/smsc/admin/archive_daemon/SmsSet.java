package ru.novosoft.smsc.admin.archive_daemon;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Совокупность записей ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class SmsSet {

  private ArrayList<SmsRow> rows = new ArrayList<SmsRow>();
  private boolean hasMore = false;

  public int getRowsCount() {
    return rows.size();
  }

  public List<SmsRow> getRowsList() {
    return rows;
  }

  public void addRow(SmsRow row) {
    rows.add(row);
  }

  public void addAll(Collection<SmsRow> smes) {
    rows.addAll(smes);
  }

  public void removeRow(SmsRow row) {
    rows.remove(row);
  }

  public void clean() {
    rows.clear();
    hasMore = false;
  }

  public boolean isHasMore() {
    return (rows.size() > 0) && hasMore;
  }

  public void setHasMore(boolean hasMore) {
    this.hasMore = hasMore;
  }
}
