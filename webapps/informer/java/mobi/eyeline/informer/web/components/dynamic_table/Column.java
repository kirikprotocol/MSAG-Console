package mobi.eyeline.informer.web.components.dynamic_table;

/**
 * @author Artem Snopkov
 */
abstract class Column  {

  private String name;
  private int width = 49;
  private String title;

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public int getWidth() {
    return width;
  }

  public void setWidth(int width) {
    this.width = width;
  }

  public String getTitle() {
    return title;
  }

  public void setTitle(String title) {
    this.title = title;
  }
}
