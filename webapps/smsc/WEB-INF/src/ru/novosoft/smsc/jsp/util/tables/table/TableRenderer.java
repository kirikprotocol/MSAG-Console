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

  public static String renderCell( Cell cell, int rowNUm, int colNum) {
    if (cell instanceof DataCell)
      return renderDataCell((DataCell)cell, rowNUm, colNum);
    else if (cell instanceof InputCell)
      return renderInputCell((InputCell)cell);;

    return "";
  }

  private static String renderDataCell(DataCell dataCell, int rowNum, int colNum) {
    final StringBuffer buffer = new StringBuffer();
    if (dataCell instanceof StringCell)
      buffer.append("<td class=name>");

    if (dataCell.isSelectable())
      buffer.append("<a href=\"#\" onclick=\"return setSelectedCell(").append(rowNum).append(",").append(colNum).append(")\">");

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
      buffer.append("<td class=check><input class=check type=checkbox name=").append(cbCell.getName()).append(" ").append(cbCell.isChecked() ? "checked" : "").append("></td>");
    }

    return buffer.toString();
  }





  // Render column -----------------------------------------------------------------------------------------------------

  public static String renderColumn(Column column, int colNum) {
    if (column instanceof TextColumn)
      return renderTextColumn((TextColumn)column, colNum);
    else if (column instanceof ImageColumn)
      return renderImageColumn((ImageColumn)column, colNum);

    return "";
  }

  private static String startSelectableColumn(int colNum) {
    return new StringBuffer().append("<a href=\"#\" onclick='return selectColumn(\"").append(colNum).append("\")'>").toString();
  }

  private static String endSelectableColumn() {
    return "</a>";
  }

  private static String renderTextColumn(TextColumn column, int colNum) {
    final StringBuffer buffer = new StringBuffer();
    if (column.isSortable())
      buffer.append(startSelectableColumn(colNum));

    if (column.getOrderType() != null)
      buffer.append("<img src=\"").append("/images/sort_").append(column.getOrderType() == OrderType.ASC ? "down" : "up").append(".gif\" class=ico16 alt=\"\">");

    String str = getLocalMessage(column.getTitle());
    if (str == null || str.equals(""))
      str = "&nbsp";
    buffer.append(str);

    if (column.isSortable())
      buffer.append(endSelectableColumn());

    return buffer.toString();
  }

  private static String renderImageColumn(ImageColumn column, int colNum) {
    final StringBuffer buffer = new StringBuffer();
    if (column.isSelectable())
      buffer.append(startSelectableColumn(colNum));

    buffer.append("<img src=\"").append(column.getImage()).append("\" class=ico16 alt=\"").append(column.getAlt()).append("\">");

    if (column.isSelectable())
      buffer.append(endSelectableColumn());

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
