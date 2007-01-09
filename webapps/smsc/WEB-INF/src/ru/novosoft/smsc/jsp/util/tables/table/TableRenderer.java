package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.jsp.util.tables.table.cell.StringCell;
import ru.novosoft.smsc.jsp.util.tables.table.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.tables.table.column.sortable.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.table.column.ImageColumn;
import ru.novosoft.smsc.util.LocaleMessages;

import java.util.ResourceBundle;
import java.util.Locale;
import java.util.MissingResourceException;

/**
 * User: artem
 * Date: 21.12.2006
 */
public class TableRenderer {

  // Render cell -------------------------------------------------------------------------------------------------------

  public static String renderCell(Cell cell, Column column) {
    if (cell instanceof DataCell)
      return renderDataCell(column, (DataCell)cell);
    else if (cell instanceof InputCell)
      return renderInputCell((InputCell)cell);;

    return "";
  }

  private static String renderDataCell(Column column, DataCell dataCell) {
    final StringBuffer buffer = new StringBuffer();
    if (dataCell instanceof StringCell)
      buffer.append("<td class=name>");

    if (dataCell.isSelectable())
      buffer.append("<a href=\"#\" onclick='return selectCell(\"").append(column.getId()).append("\",\"").append(dataCell.getId()).append("\")'>");

    if (dataCell instanceof StringCell) {
      String str = ((StringCell)dataCell).getStr();
      if (str == null || str.equals(""))
        str = "&nbsp";

      buffer.append(str);
    }

    if (dataCell.isSelectable())
      buffer.append("</a>");

    buffer.append("</td>");
    return buffer.toString();
  }



  private static String renderInputCell(InputCell inputCell) {
    final StringBuffer buffer = new StringBuffer();

    if (inputCell instanceof CheckBoxCell) {
      final CheckBoxCell cbCell = (CheckBoxCell)inputCell;
      buffer.append("<td class=check><input class=check type=checkbox name=").append(cbCell.getId()).append(" ").append(cbCell.isChecked() ? "checked" : "").append("></td>");
    }

    return buffer.toString();
  }





  // Render column -----------------------------------------------------------------------------------------------------

  public static String renderColumn(Column column, OrderType orderType) {
    final StringBuffer buffer = new StringBuffer();

    if (column.isSortable())
      buffer.append(startSortableColumn(column));

    if (orderType != null)
      buffer.append("<img src=\"").append("/images/sort_").append(orderType == OrderType.ASC ? "down" : "up").append(".gif\" class=ico16 alt=\"\">");

    if (column instanceof TextColumn)
      buffer.append(renderTextColumn((TextColumn)column));
    else if (column instanceof ImageColumn)
      buffer.append(renderImageColumn((ImageColumn)column));

    if (column.isSortable())
      buffer.append(endSortableColumn());

    return buffer.toString();
  }

  private static String startSortableColumn(Column column) {
    return new StringBuffer().append("<a href=\"#\" onclick='return selectColumn(\"").append(column.getId()).append("\")'>").toString();
  }

  private static String endSortableColumn() {
    return "</a>";
  }

  private static String renderTextColumn(TextColumn column) {
    final StringBuffer buffer = new StringBuffer();

    String str = getLocalMessage(column.getTitle());
    if (str == null || str.equals(""))
      str = "&nbsp";
    buffer.append(str);

    return buffer.toString();
  }

  private static String renderImageColumn(ImageColumn column) {
    final StringBuffer buffer = new StringBuffer();

    buffer.append("<img src=\"").append(column.getImage()).append("\" class=ico16 alt=\"").append(column.getAlt()).append("\">");

    return buffer.toString();
  }

  private static String getLocalMessage(String key) {
    try {
      ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
      return bundle.getString(key);
    } catch (MissingResourceException e) {
    }
    return key;
  }
}
