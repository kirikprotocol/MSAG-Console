package ru.novosoft.smsc.jsp.util.helper.dynamictable.column;

import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 18.01.2007
 */

public class TextColumn extends Column {

  private final Validation validation;
  private final boolean allowEditAfterAdd;
  private final int maxLength;


  public TextColumn(DynamicTableHelper tableHelper, String name, String uid, int width, Validation validation, boolean allowEditAfterAdd) {
    this(tableHelper, name, uid, width, -1, validation, allowEditAfterAdd);
  }

  public TextColumn(DynamicTableHelper tableHelper, String name, String uid, int width, int maxLength, Validation validation, boolean allowEditAfterAdd) {
    super(tableHelper, name, uid, width);
    this.validation = validation;
    this.maxLength = maxLength;
    this.allowEditAfterAdd = allowEditAfterAdd;
  }

  public Validation getValidation() {
    return validation;
  }

  public boolean isAllowEditAfterAdd() {
    return allowEditAfterAdd;
  }

  public Object getValue(HttpServletRequest request, int rowNumber) throws IncorrectValueException {
    return request.getParameter(getCellParameterName(rowNumber));
  }

  public Object getBaseValue(HttpServletRequest request) throws IncorrectValueException {
    return request.getParameter(getNewCellParameterName());
  }

  public int getMaxLength() {
    return maxLength;
  }
}
