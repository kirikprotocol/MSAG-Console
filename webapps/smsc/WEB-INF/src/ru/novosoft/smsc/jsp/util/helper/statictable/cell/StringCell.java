package ru.novosoft.smsc.jsp.util.helper.statictable.cell;

import ru.novosoft.smsc.jsp.util.helper.statictable.DataCell;

/**
 * User: artem
 * Date: 18.12.2006
 */
public class StringCell extends DataCell {

  private final String string;

  public StringCell(String id, final String str, final boolean selectable) {
    super(id, selectable);
    this.string = str;
  }

  public String getStr() {
    return string;
  }
}
