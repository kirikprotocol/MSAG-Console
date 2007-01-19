package ru.novosoft.smsc.jsp.util.helper.dynamictable.column;

import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;

/**
 * User: artem
 * Date: 18.01.2007
 */
public class SelectColumn extends Column {

  private final Object[] values;
  private final boolean allowEditAfterAdd;
  private final Validation validation;

  public SelectColumn(String name, String uid, int width, boolean allowEditAfterAdd, Validation validation, Object[] values) {
    super(Type.SELECT_COLUMN, name, uid, width);
    this.allowEditAfterAdd = allowEditAfterAdd;
    this.validation = validation;

    this.values = values;
  }

  public String[] getValuesAsStrings() {
    final String[] result = new String[values.length];
    for (int i = 0; i < values.length; i++)
      result[i] = (values[i] == null) ? null : values[i].toString();
    return result;
  }

  public int getValueId(Object value) {
    for (int i=0; i<values.length; i++)
      if (values[i] == value)
        return i;

    return -1;
  }

  public boolean isAllowEditAfterAdd() {
    return allowEditAfterAdd;
  }

  public Validation getValidation() {
    return validation;
  }

  public Object getValue(String value) throws IncorrectValueException {
    try {
      return values[Integer.parseInt(value)];
    } catch (Exception e) {
      return null;
    }
  }

}
