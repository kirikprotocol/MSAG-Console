package ru.novosoft.smsc.web.components.paged_table;

import javax.faces.component.UIComponent;
import javax.faces.event.ActionEvent;

/**
 * @author Artem Snopkov
 */
public class SelectElementEvent extends ActionEvent {

  private String columnName;
  private int rowNumber;

  SelectElementEvent(UIComponent uiComponent, String columnName, int rowNumber) {
    super(uiComponent);
    this.columnName = columnName;
    this.rowNumber = rowNumber;
  }

  public String getColumnName() {
    return columnName;
  }

  public int getRowNumber() {
    return rowNumber;
  }
}
