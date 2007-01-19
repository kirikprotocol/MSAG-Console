package ru.novosoft.smsc.jsp.util.helper.dynamictable.column;

import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;

/**
 * User: artem
 * Date: 18.01.2007
 */

public class TextColumn extends Column {

  private final Validation validation;
  private final boolean allowEditAfterAdd;
  private final int maxLength;


  public TextColumn(String name, String uid, int width, Validation validation, boolean allowEditAfterAdd) {
    this(name, uid, width, -1, validation, allowEditAfterAdd);
  }

  public TextColumn(String name, String uid, int width, int maxLength, Validation validation, boolean allowEditAfterAdd) {
    super(Type.TEXT_COLUMN, name, uid, width);
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

  public Object getValue(String valueId) throws IncorrectValueException {
    return valueId;
  }

  public int getMaxLength() {
    return maxLength;
  }
}
