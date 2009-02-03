package ru.novosoft.smsc.jsp.util.helper.dynamictable;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * User: artem
 * Date: 16.01.2007
 */

public abstract class DynamicTableHelper {

  public static final String TOTAL_COUNT_PREFIX = "_total_count_prefix";

  private final ArrayList columns = new ArrayList();
  private final ArrayList rows = new ArrayList();

  private final String name;
  private final String uid;
  private boolean showColumnsTitle = true;
  private boolean showTableTitle = true;
  private boolean allowAddValues = true;
  private Row lastRow = new Row();

  public DynamicTableHelper(String name, String uid) {
    this.name = name;
    this.uid = uid;
  }


//  Request processing =================================================================================================


  public void processRequest(HttpServletRequest request) throws IncorrectValueException {
    if (request.getParameter(getTotalCountPrefix()) == null)
      return;

    int totalCount = Integer.parseInt(request.getParameter(getTotalCountPrefix()));
    for (int i = 0; i < totalCount ; i++)
      addRow(readRow(request, i));

    lastRow = readLastRow(request);
  }

  private void addRow(Row row) {
    if (row != null && !row.isEmpty())
      rows.add(row);
  }

  private Row readRow(HttpServletRequest request, int rowNum) throws IncorrectValueException {
    final Row row = new Row();
    for (int i=0; i< columns.size(); i++) {
      final Column column = (Column)columns.get(i);
      row.addValue(column, column.getValue(request, rowNum));
    }
    return row;
  }

  private Row readLastRow(HttpServletRequest request) throws IncorrectValueException {
    final Row row = new Row();
    for (int i=0; i< columns.size(); i++) {
      final Column column = (Column)columns.get(i);
      row.addValue(column, column.getBaseValue(request));
    }
    return row;
  }


//  Abstract ===========================================================================================================


  protected abstract void fillTable();


//  Methods ============================================================================================================


  public void clear() {
    rows.clear();
    lastRow = new Row();
  }

  public void setShowColumnsTitle(boolean showColumnsTitle) {
    this.showColumnsTitle = showColumnsTitle;
  }

  public boolean isShowColumnsTitle() {
    return showColumnsTitle;
  }

  public boolean isAllowAddValues() {
    return allowAddValues;
  }

  public void setAllowAddValues(boolean allowAddValues) {
    this.allowAddValues = allowAddValues;
  }

  public boolean isShowTableTitle() {
    return showTableTitle;
  }

  public void setShowTableTitle(boolean showTableTitle) {
    this.showTableTitle = showTableTitle;
  }

  //  Table creation =====================================================================================================


  protected Row createNewRow() {
    final Row newRow = new Row();
    rows.add(newRow);
    return newRow;
  }

  protected void addColumn(Column column) {
    if (column != null)
      columns.add(column);
  }


//  Prefixes ===========================================================================================================


  public String getTotalCountPrefix() {
    return uid + TOTAL_COUNT_PREFIX;
  }


//  Properties =========================================================================================================


  public String getName() {
    return name;
  }

  public String getUid() {
    return uid;
  }

  public int getRowsCount() {
    return rows.size();
  }

  public Iterator getColumns() {
    return columns.iterator();
  }

  public int getColumnsCount() {
    return columns.size();
  }

  protected Iterator getRows() {
    return rows.iterator();
  }

  public Row getLastRow() {
    return lastRow;
  }

}
