package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.BeanWithTemporaryContent;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 18.12.2006
 */

public abstract class SimpleTableBean extends BeanWithTemporaryContent {

  private ArrayList columns = new ArrayList();
  private ArrayList rows = new ArrayList();
  private ArrayList inputCells = new ArrayList();

  private int selectedColumn = -1;
  private int selectedRow = -1;

  private int column = -1;

  public final int processRequest(HttpServletRequest request) {
    int result = RESULT_OK;

    if (hasTemporaryContent()) {
      rows = (ArrayList)getFromTemporaryContent("rows");
      inputCells = (ArrayList)getFromTemporaryContent("inputCells");
    }

    setDataToInputCells(request);

    result = checkDataCellSelected(request);
    if (result != RESULT_OK)
      return result;

    result = checkColumnSelected(request);
    if (result != RESULT_OK)
      return result;

    result = processRequestInternal(request);
    if (result != RESULT_OK)
      return result;

    try {
      fillTable();
    } catch (AdminException e) {
      logger.error("Can't fill table", e);
      return _error(new SMSCJspException("Can't fill table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    addToTemporaryContent("rows", rows);
    addToTemporaryContent("inputCells", inputCells);

    return result;
  }

  protected void clear() {
    rows.clear();
    inputCells.clear();
  }

  private void setDataToInputCells(HttpServletRequest request) {
    for(Iterator iter = inputCells.iterator(); iter.hasNext();) {
      final InputCell inputCell = (InputCell)iter.next();
      inputCell.setValue(request.getParameter(inputCell.getName()));
    }
  }

  protected int checkDataCellSelected(HttpServletRequest request) {
    int result = RESULT_OK;
    if (selectedColumn >= 0 && selectedColumn < columns.size() && selectedRow >= 0 && selectedRow < rows.size()) {
      final Column column = (Column)columns.get(selectedColumn);
      final Row row = ((Row)rows.get(selectedRow));
      final Cell cell = row.getCell(column);
      System.out.println(cell);
      if (cell instanceof DataCell)
        result = onCellSelected(column, row, (DataCell)cell);
    }

    selectedColumn = -1;
    selectedRow = -1;

    return result;
  }

  protected int checkColumnSelected(HttpServletRequest request) {
    int result = RESULT_OK;
    if (column >= 0 && column < columns.size())
      result = onColumnSelected((Column)columns.get(column));

    column = -1;
    return result;
  }

  protected abstract int processRequestInternal(HttpServletRequest request);

  protected abstract void fillTable() throws AdminException ;

  protected abstract int onCellSelected(final Column column, final Row row, final DataCell cell);

  protected abstract int onColumnSelected(final Column column);

  String addColumn(final Column column) {
    if (column != null) {
      columns.add(column);
      return String.valueOf(columns.size());
    }
    return null;
  }

  public Iterator getColumns() {
    return columns.iterator();
  }

  public List getCellsByColumn(Column column) {
    final ArrayList result = new ArrayList();
    for (int i = 0; i < rows.size(); i++) {
      Row row = (Row) rows.get(i);
      result.add(row.getCell(column));
    }
    return result;
  }



  protected final Row createNewRow() {
    final Row row = new Row();
    rows.add(row);
    return row;
  }

  public Iterator getRows() {
    return rows.iterator();
  }


  final void registerCell(final InputCell cell) {
    if (cell != null && cell.getName() != null)
      inputCells.add(cell);
  }

  public int getSize() {
    return rows.size();
  }


  public void setSelectedColumn(String column) {
    try {
      selectedColumn = Integer.parseInt(column);
    } catch (NumberFormatException e) {
      logger.error("Can't set selected column", e);
      selectedColumn = -1;
    }
  }

  public void setSelectedRow(String row) {
    try {
      selectedRow = Integer.parseInt(row);
    } catch (NumberFormatException e) {
      logger.error("Can't set selected row", e);
      selectedRow = -1;
    }
  }

  public void setColumn(String column) {
    try {
      this.column = Integer.parseInt(column);
    } catch (NumberFormatException e) {
      logger.error("Can't set column", e);
      this.column = -1;
    }
  }

}
