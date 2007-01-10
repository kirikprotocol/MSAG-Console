package ru.novosoft.smsc.jsp.util.tables.table.cell;

import ru.novosoft.smsc.jsp.util.tables.table.DataCell;

/**
 * User: artem
 * Date: 09.01.2007
 */
public class ImageCell extends DataCell{

  private final String image;

  public ImageCell(String id, String image, boolean selectable) {
    super(id, selectable);
    this.image = image;
  }

  public String getImage() {
    return image;
  }
}
