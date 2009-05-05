package ru.novosoft.smsc.jsp.util.helper.statictable.cell;

import ru.novosoft.smsc.jsp.util.helper.statictable.DataCell;

/**
 * User: artem
 * Date: 09.01.2007
 */
public class ImageCell extends DataCell{

  private final String image;
  private final String text;

  public ImageCell(String id, String image, boolean selectable) {
    this(id, image, null, selectable);
  }

  public ImageCell(String id, String image, String text, boolean selectable) {
    super(id, selectable);
    this.image = image;
    this.text = text;
  }

  public String getImage() {
    return image;
  }

  public String getText() {
    return text;
  }
}
