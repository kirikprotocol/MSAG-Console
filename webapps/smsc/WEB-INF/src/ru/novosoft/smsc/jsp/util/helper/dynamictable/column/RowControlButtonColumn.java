package ru.novosoft.smsc.jsp.util.helper.dynamictable.column;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 18.01.2007
 */

public class RowControlButtonColumn extends Column {

  private final boolean allowRemoveAddedRows;

  public RowControlButtonColumn(DynamicTableHelper tableHelper, String name, String uid) {
    this(tableHelper, name, uid, 1, true);
  }

  public RowControlButtonColumn(DynamicTableHelper tableHelper, String name, String uid, int width, boolean allowRemoveAddedRows) {
    super(tableHelper, name, uid, width);
    this.allowRemoveAddedRows = allowRemoveAddedRows;
  }

  public boolean isAllowRemoveAddedRows() {
    return allowRemoveAddedRows;
  }

  public Object getValue(HttpServletRequest request, int rowNumber) throws IncorrectValueException {
    return null;
  }

  public Object getBaseValue(HttpServletRequest request) throws IncorrectValueException {
    return null;
  }
}
