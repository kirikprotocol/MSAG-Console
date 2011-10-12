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
  private int maxRows = 0;
  private int smesRows = 0;

  public int getRowsCount() {
    return rows.size();
  }

  public int getSmesRows() {
    return smesRows;
  }

  public void setSmesRows(int smesRows) {
    this.smesRows = smesRows;
  }

  public List getRowsList() {
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

  public SmsRow getRow(int index) {
    return rows.get(index);
  }

  public SmsRow getRow(String id) {
    SmsRow pattern = new SmsRow();
    pattern.setId(Long.parseLong(id));
    int index = rows.indexOf(pattern);
    return (index >= 0) ? getRow(index) : null;
  }

  public SmsRow getRow(long id) {
    SmsRow pattern = new SmsRow();
    pattern.setId(id);
    int index = rows.indexOf(pattern);
    return (index >= 0) ? getRow(index) : null;
  }

  public boolean isHasMore() {
    return (rows.size() > 0) && hasMore;
  }

  public void setHasMore(boolean hasMore) {
    this.hasMore = hasMore;
  }
}
