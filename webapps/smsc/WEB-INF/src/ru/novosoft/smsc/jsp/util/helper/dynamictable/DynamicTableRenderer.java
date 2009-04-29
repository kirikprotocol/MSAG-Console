package ru.novosoft.smsc.jsp.util.helper.dynamictable;

import ru.novosoft.smsc.jsp.util.helper.HTMLRenderer;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.SelectColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;

import java.util.Iterator;

/**
 * User: artem
 * Date: 16.01.2007
 */

public class DynamicTableRenderer extends HTMLRenderer {

  public static final byte EDIT = 1;

  public static String renderTableBody(DynamicTableHelper tableHelper) {
    int i=0;
    final StringBuffer buffer = new StringBuffer();
    for (Iterator rows = tableHelper.getRows(); rows.hasNext(); i++) {
      final Row row = (rows.hasNext()) ? (Row)rows.next() : null;
      buffer.append(renderRow(tableHelper, row, i));
    }
    
    buffer.append(renderLastRow(tableHelper, tableHelper.getLastRow(), i));
    return buffer.toString();
  }

  private static String renderRow(DynamicTableHelper tableHelper, Row row, int rowNum) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append(trStart("row" + (rowNum&1), "row_" + rowNum));
    {
      for (Iterator cols = tableHelper.getColumns(); cols.hasNext();) {
        final Column column = (Column)cols.next();

        buffer.append(tdStart(column.getWidth() + "%"));
        buffer.append(renderColumnRow(tableHelper, column, row, rowNum));
        buffer.append(tdEnd());
      }
      buffer.append(tdStart()).append(tdEnd());
    }
    buffer.append(trEnd());

    return buffer.toString();
  }

  private static String renderLastRow(DynamicTableHelper tableHelper, Row row, int rowNum) {
    if (!tableHelper.isAllowAddValues())
      return "";

    final String uid = tableHelper.getUid();

    final StringBuffer buffer = new StringBuffer();
    buffer.append(trStart("row" + (rowNum&1), uid + "newrow"));
    {
      for (Iterator iter = tableHelper.getColumns(); iter.hasNext();) {
        final Column column = (Column)iter.next();

        buffer.append(tdStart(column.getWidth() + "%"));
        buffer.append(renderColumnLastRow(tableHelper, column, row));
        buffer.append(tdEnd());
      }
      buffer.append(tdStart()).append(tdEnd());
    }
    buffer.append(trEnd());

    return buffer.toString();
  }



  private static String renderColumnRow(DynamicTableHelper tableHelper, Column column, Row row, int rowNum) {
    String uid = tableHelper.getUid();

    if (column instanceof TextColumn) {
      final TextColumn textColumn = (TextColumn)column;
      return textInput("txtW",
                       column.getCellParameterName(rowNum),
                       column.getCellParameterName(rowNum),
                       (row != null) ? (String)row.getValue(column) : "",
                       !textColumn.isAllowEditAfterAdd(),
                       textColumn.getMaxLength(), null,
                       textColumn.getValidation(), null);

    } else if (column instanceof RowControlButtonColumn) {
      final RowControlButtonColumn rowColumn = (RowControlButtonColumn)column;
      if (rowColumn.isAllowRemoveAddedRows())
        return image("/images/but_del.gif", "delRow('" + uid + "'," + rowNum + ")", getLocalMessage("common.hints.delParam", tableHelper.getLocale()));
      else return "";
    } else if (column instanceof SelectColumn) {
      final SelectColumn selectColumn = (SelectColumn)column;
      return select("selectW",
                    column.getCellParameterName(rowNum),
                    column.getCellParameterName(rowNum),
                    selectColumn.getValuesAsStrings(),
                    selectColumn.getValueId(row.getValue(column)),
                    !selectColumn.isAllowEditAfterAdd(),
                    selectColumn.getValidation());
    }

    return "";
  }

  private static String generateStringArray(String[] array) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("new Array(");

    for (int i=0; i< array.length; i++)
      buffer.append("'").append(array[i]).append("'").append((i+1<array.length) ? ", " : "");

    buffer.append(")");
    return buffer.toString();
  }

  private static String generateArray(String[] array) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("new Array(");

    for (int i=0; i< array.length; i++)
      buffer.append(array[i]).append((i+1<array.length) ? ", " : "");

    buffer.append(")");
    return buffer.toString();
  }

  private static String getColumnJavaScriptClass(Column column) {
    final StringBuffer buffer = new StringBuffer();
    if (column instanceof TextColumn) {
      final TextColumn textColumn = (TextColumn)column;
      buffer.append("new textColumn('").append(column.getUid()).append("',")
          .append(textColumn.getValidation() == null ? "null" : "'" + textColumn.getValidation().getValidation() + "'")
          .append(",").append(textColumn.isAllowEditAfterAdd()).append(")");
    } else if (column instanceof RowControlButtonColumn) {
      final RowControlButtonColumn rowColumn = (RowControlButtonColumn)column;
      buffer.append("new rowControlButtonColumn('").append(column.getUid()).append("',").append(rowColumn.isAllowRemoveAddedRows()).append(")");
    } else if (column instanceof SelectColumn) {
      final SelectColumn selectColumn = (SelectColumn)column;
      buffer.append("new selectColumn('").append(column.getUid()).append("',")
          .append(selectColumn.getValidation() == null ? "null" : "'" + selectColumn.getValidation().getValidation() + "'")
          .append(",").append(generateStringArray(selectColumn.getValuesAsStrings())).append(",").append(selectColumn.isAllowEditAfterAdd()).append(")");
    }
    return buffer.toString();
  }

  private static String generateColumns(DynamicTableHelper tableHelper) {
    final String[] result = new String[tableHelper.getColumnsCount()];
    int i=0;
    for (Iterator iter = tableHelper.getColumns(); iter.hasNext();i++)
      result[i] = getColumnJavaScriptClass((Column)iter.next());

    return generateArray(result);
  }

  private static String renderColumnLastRow(DynamicTableHelper tableHelper, Column column, Row row) {
    String uid = tableHelper.getUid();

    if (column instanceof TextColumn) {
      final TextColumn textColumn = (TextColumn)column;
      return textInput("txtW",
                       column.getNewCellParameterName(),
                       column.getNewCellParameterName(),
                       (row != null) ? (String)row.getValue(column) : "",
                       false,
                       textColumn.getMaxLength(), null,
                       null, null);

    } else if (column instanceof RowControlButtonColumn) {
      final RowControlButtonColumn rowColumn = (RowControlButtonColumn)column;
      if (rowColumn.isAllowRemoveAddedRows())
        return image("/images/but_add.gif", "addRow('" + uid + "','" + tableHelper.getTotalCountPrefix() + "'," + generateColumns(tableHelper)+ ")", getLocalMessage("common.hints.addParam", tableHelper.getLocale()), "button");
      else return "";
    } else if (column instanceof SelectColumn) {
      final SelectColumn selectColumn = (SelectColumn)column;
      return select("selectW",
                    column.getNewCellParameterName(),
                    column.getNewCellParameterName(),
                    selectColumn.getValuesAsStrings(),
                    0,
                    false,
                    null);
    }

    return "";
  }
}
