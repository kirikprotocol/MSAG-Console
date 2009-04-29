package ru.novosoft.smsc.jsp.util.helper.statictable;

import ru.novosoft.smsc.jsp.util.helper.HTMLRenderer;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.ImageCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.ImageColumn;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;

import java.util.Locale;

/**
 * User: artem
 * Date: 21.12.2006
 */

public class StaticTableRenderer extends HTMLRenderer {

  // Render cell =======================================================================================================

  public static String renderCell(StaticTableHelper helper, Cell cell, Column column) {
    if (cell instanceof DataCell)
      return renderDataCell(helper, column, (DataCell)cell);
    else if (cell instanceof InputCell)
      return renderInputCell((InputCell)cell);;

    return "";
  }

  private static String renderDataCell(StaticTableHelper helper, Column column, DataCell dataCell) {
    final StringBuffer buffer = new StringBuffer();
//    if (dataCell instanceof StringCell)
    buffer.append(tdStart("name", null));

    if (dataCell.isSelectable())
      buffer.append(aStart("#", "return selectCell('" + helper.getSelectedColumnIdPrefix() +  "','" + column.getId() + "','" + helper.getSelectedCellIdPrefix() + "','" + dataCell.getId() + "');"));

    if (dataCell instanceof StringCell) {
      String str = ((StringCell)dataCell).getStr();
      if (str == null || str.equals(""))
        str = "&nbsp";

      buffer.append(str);
    } else if (dataCell instanceof ImageCell) {
      String image = ((ImageCell)dataCell).getImage();
      if (image != null) {
        buffer.append(image(image, null, null));
      } else
        buffer.append("&nbsp;");
    }

    if (dataCell.isSelectable())
      buffer.append(aEnd());

    buffer.append(tdEnd());
    return buffer.toString();
  }



  private static String renderInputCell(InputCell inputCell) {
    final StringBuffer buffer = new StringBuffer();

    if (inputCell instanceof CheckBoxCell) {
      final CheckBoxCell cbCell = (CheckBoxCell)inputCell;
      buffer.append(tdStart("check", null));
      buffer.append(check(cbCell.getId(), cbCell.isChecked()));
      buffer.append(tdEnd());
    }

    return buffer.toString();
  }



  // Render column =====================================================================================================

  public static String renderColumn(StaticTableHelper helper, Column column, OrderType orderType) {
    final StringBuffer buffer = new StringBuffer();

    if (column.isSortable())
      buffer.append(startSortableColumn(helper, column));

    if (orderType != null)
      buffer.append(image("/images/sort_" + (orderType == OrderType.ASC ? "down" : "up") + ".gif", null, null, "ico16"));

    if (column instanceof TextColumn)
      buffer.append(renderTextColumn((TextColumn)column, helper.getLocale()));
    else if (column instanceof ImageColumn)
      buffer.append(renderImageColumn((ImageColumn)column));

    if (column.isSortable())
      buffer.append(endSortableColumn());

    return buffer.toString();
  }

  private static String startSortableColumn(StaticTableHelper helper, Column column) {
    return aStart("#", "return sortColumn('" + helper.getSortedColumnIdPrefix() + "','" + column.getId() + "');");
  }

  private static String endSortableColumn() {
    return aEnd();
  }

  private static String renderTextColumn(TextColumn column, Locale locale) {
    final StringBuffer buffer = new StringBuffer();

    String str = getLocalMessage(column.getTitle(), locale);
    if (str == null || str.equals(""))
      str = "&nbsp";
    buffer.append(str);

    return buffer.toString();
  }

  private static String renderImageColumn(ImageColumn column) {
    final StringBuffer buffer = new StringBuffer();

    buffer.append(image(column.getImage(), null, column.getAlt()));

    return buffer.toString();
  }

}
