package ru.novosoft.smsc.jsp.util.helper.dynamictable.column;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;

/**
 * User: artem
 * Date: 18.01.2007
 */

public class RowControlButtonColumn extends Column {

  private final boolean allowRemoveAddedRows;

  public RowControlButtonColumn(String name, String uid) {
    this(name, uid, 1, true);
  }

  public RowControlButtonColumn(String name, String uid, int width, boolean allowRemoveAddedRows) {
    super(Type.ROW_CONTROL_BUTTON_COLUMN, name, uid, width);
    this.allowRemoveAddedRows = allowRemoveAddedRows;
  }

  public boolean isAllowRemoveAddedRows() {
    return allowRemoveAddedRows;
  }

  public Object getValue(String valueId) throws IncorrectValueException {
    return null;
  }
}
