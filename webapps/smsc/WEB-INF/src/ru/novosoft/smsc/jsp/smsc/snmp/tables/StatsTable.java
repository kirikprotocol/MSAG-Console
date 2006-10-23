package ru.novosoft.smsc.jsp.smsc.snmp.tables;

import java.util.*;

/**
 * User: artem
 * Date: 16.10.2006
 */

public class StatsTable {

  private ArrayList rows;
  private final StatsTableScheme scheme;

  public StatsTable(final StatsTableScheme scheme) {
    rows = new ArrayList();
    this.scheme = scheme;
  }

  public StatsTableRow createRow() {
    final StatsTableRow row = new StatsTableRow(scheme);
    rows.add(row);
    return row;
  }

  public void orderBy(String fieldName, boolean asc) {
    if (!scheme.hasField(fieldName))
      throw new IllegalArgumentException("Table's scheme has not field: " + fieldName);

    final Object[] values = rows.toArray();
    Arrays.sort(values, new StatsTableComparator(fieldName, asc));
    rows.clear();
    rows.addAll(Arrays.asList(values));
  }

  public StatsTableRow insertRow(StatsTableRow afterRow) {
    final StatsTableRow row = new StatsTableRow(scheme);
    final int index = rows.indexOf(afterRow);
    if (index < 0)
      throw new IllegalArgumentException("Row not found in table");
    rows.add(index+1, row);;
    return row;
  }

  public void deleteRow(StatsTableRow row) {
    rows.remove(row);
  }

  public Iterator getRows() {
    return rows.iterator();
  }

  public StatsTableRow getRow(int number) {
    if (number >= getSize())
      return null;

    return (StatsTableRow)rows.get(number);
  }

  public int getSize() {
    return rows.size();
  }

  public void clear() {
    for (Iterator iter = rows.iterator(); iter.hasNext();)
      ((StatsTableRow)iter.next()).clear();

    rows.clear();
  }

  public StatsTableScheme getScheme() {
    return scheme;
  }


  private class StatsTableComparator implements Comparator {

    private final String orderBy;
    private final boolean asc;

    StatsTableComparator(final String orderBy, final boolean asc) {
      this.orderBy = orderBy;
      this.asc = asc;
    }

    public int compare(Object o1, Object o2) {
      if (orderBy == null)
        return -1;

      final StatsTableRow row1 = (StatsTableRow)o1;
      final StatsTableRow row2 = (StatsTableRow)o2;

      return (asc) ? row1.getValue(orderBy).compareTo(row2.getValue(orderBy)) : -row1.getValue(orderBy).compareTo(row2.getValue(orderBy));
    }
  }
}
