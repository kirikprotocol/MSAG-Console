package ru.novosoft.smsc.jsp.util.tables.table.cell;

import ru.novosoft.smsc.jsp.util.tables.table.InputCell;
import ru.novosoft.smsc.jsp.util.tables.table.SimpleTableBean;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class CheckBoxCell extends InputCell{

  private boolean checked = false;

  public CheckBoxCell(SimpleTableBean bean, Object object, boolean checked) {
    super(bean, object);
    this.checked = checked;
  }

  public void setValue(String value) {
    if (value == null)
      return;

    checked = (value.equalsIgnoreCase("true") || value.equalsIgnoreCase("on"));
  }

  public boolean isChecked() {
    return checked;
  }
}
