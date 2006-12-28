package ru.novosoft.smsc.jsp.util.tables.table.cell;

import ru.novosoft.smsc.jsp.util.tables.table.DataCell;

/**
 * User: artem
 * Date: 18.12.2006
 */
public class StringCell extends DataCell {

  private final String string;

  public StringCell(final String str, final Object value, final boolean selectable) {
    super(value, selectable);
    this.string = str;
  }

  public String getStr() {
    return string;
  }
}
