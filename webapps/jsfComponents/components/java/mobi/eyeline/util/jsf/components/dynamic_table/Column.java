package mobi.eyeline.util.jsf.components.dynamic_table;

/**
 * @author Artem Snopkov
 */
abstract class Column {

  private String name;
  private String columnClass;
  private String title;

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getColumnClass() {
    return columnClass;
  }

  public void setColumnClass(String columnClass) {
    this.columnClass = columnClass;
  }

  public String getTitle() {
    return title;
  }

  public void setTitle(String title) {
    this.title = title;
  }
}
