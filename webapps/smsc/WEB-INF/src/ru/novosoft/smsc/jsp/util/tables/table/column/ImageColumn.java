package ru.novosoft.smsc.jsp.util.tables.table.column;

import ru.novosoft.smsc.jsp.util.tables.table.Column;
import ru.novosoft.smsc.jsp.util.tables.table.SimpleTableBean;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class ImageColumn extends Column{
  private final String image;
  private final String alt;

  public ImageColumn(SimpleTableBean simpleTableBean, String image, String alt) {
    super(simpleTableBean);
    this.image = image;
    this.alt = alt;
  }

  public ImageColumn(SimpleTableBean simpleTableBean, String image, String alt, boolean selectable) {
    super(simpleTableBean, selectable);
    this.image = image;
    this.alt = alt;
  }

  public ImageColumn(SimpleTableBean simpleTableBean, String image, String alt, boolean selectable, int width) {
    super(simpleTableBean, selectable, width);
    this.image = image;
    this.alt = alt;
  }

  public String getImage() {
    return image;
  }

  public String getAlt() {
    return alt;
  }
}
