package ru.novosoft.smsc.web.components.paged_table;

/**
 * @author Artem Snopkov
 */
public class TextColumn extends Column {    

  private String textColor;

  public TextColumn(String name, String title) {
    super(name, title);
  }

  public String getTextColor() {
    return textColor;
  }

  public void setTextColor(String textColor) {
    this.textColor = textColor;
  }
}
