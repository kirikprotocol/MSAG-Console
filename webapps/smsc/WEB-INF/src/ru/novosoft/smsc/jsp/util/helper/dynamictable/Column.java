package ru.novosoft.smsc.jsp.util.helper.dynamictable;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 16.01.2007
 */

public abstract class Column {

  public static final String NEW_CELL_PREFIX = "_newcell_";
  public static final String CELL_PREFIX = "_cell_";

  private final String name;
  private final String uid;
  private final int width;
  private final DynamicTableHelper tableHelper;

  protected Column(DynamicTableHelper tableHelper, String name, String uid, int width) {
    this.tableHelper = tableHelper;
    this.name = name;
    this.uid = uid;
    this.width = width;
  }

  public String getName() {
    return name;
  }

  public String getUid() {
    return uid;
  }

  public int getWidth() {
    return width;
  }

  public String getNewCellParameterName() {
    return tableHelper.getUid() + NEW_CELL_PREFIX + uid;
  }

  public String getCellParameterName(int rowNum) {
    return tableHelper.getUid() + CELL_PREFIX + uid + rowNum;
  }

  public abstract Object getValue(HttpServletRequest request, int rowNumber) throws IncorrectValueException;

  public abstract Object getBaseValue(HttpServletRequest request) throws IncorrectValueException;

}
