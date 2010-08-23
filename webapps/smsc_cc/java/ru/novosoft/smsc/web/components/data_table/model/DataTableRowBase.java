package ru.novosoft.smsc.web.components.data_table.model;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DataTableRowBase implements DataTableRow {

  private final String id;
  private final Object data;
  private final Object innerData;
  private List<Object> innerRows;

  public DataTableRowBase(String id, Object data, Object innerData) {
    this.id = id;
    this.data = data;
    this.innerData = innerData;
  }

  public String getId() {
    return id;
  }

  public Object getData() {
    return data;
  }

  public Object getInnerData() {
    return innerData;
  }

  public List<Object> getInnerRows() {
    return innerRows;
  }

  public void setInnerRows(List<Object> rows) {
    this.innerRows = rows;
  }

  public void addInnerRow(Object row) {
    if (innerRows == null)
      innerRows = new ArrayList<Object>();
    innerRows.add(row);
  }
}
