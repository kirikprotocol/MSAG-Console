package ru.novosoft.smsc.web.components.dynamic_table;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class TableModel implements Serializable {

  private List<TableRow> rows = new ArrayList<TableRow>();

  public void addRow(TableRow row) {
    rows.add(row);
  }

  public List<TableRow> getRows() {
    return rows;
  }

  public int getRowCount() {
    return rows.size();
  }
}
