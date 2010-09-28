package mobi.eyeline.informer.admin.infosme;

/**
 * @author Aleksandr Khalitov
 */
public class Category {

  private String level;

  private String name;

  public String getLevel() {
    return level;
  }

  public Category setLevel(String level) {
    this.level = level;
    return this;
  }

  public String getName() {
    return name;
  }

  public Category setName(String name) {
    this.name = name;
    return this;
  }
}
