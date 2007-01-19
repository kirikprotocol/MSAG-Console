package ru.novosoft.smsc.jsp.util.helper.statictable.column;

import ru.novosoft.smsc.jsp.util.helper.statictable.Column;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class ImageColumn extends Column {
  private final String image;
  private final String alt;

  public ImageColumn(String uId, String image, String alt) {
    super(uId);
    this.image = image;
    this.alt = alt;
  }

  public ImageColumn(String uId, String image, String alt, boolean sortable) {
    super(uId, sortable);
    this.image = image;
    this.alt = alt;
  }

  public ImageColumn(String uId, String image, String alt, boolean sortable, int width) {
    super(uId, sortable, width);
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
