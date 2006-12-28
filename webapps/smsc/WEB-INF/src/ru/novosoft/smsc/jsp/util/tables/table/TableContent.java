package ru.novosoft.smsc.jsp.util.tables.table;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 27.12.2006
 */
public class TableContent {
  private final String name;
  private final ArrayList rows;
  private final ArrayList inputCells;

  public TableContent(String name, ArrayList rows, ArrayList inputCells) {
    this.name = name;
    this.rows = rows;
    this.inputCells = inputCells;
  }


  public ArrayList getRows() {
    return rows;
  }

  public ArrayList getInputCells() {
    return inputCells;
  }

  public String getName() {
    return name;
  }
}
