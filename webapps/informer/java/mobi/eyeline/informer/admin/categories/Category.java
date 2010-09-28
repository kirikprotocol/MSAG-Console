package mobi.eyeline.informer.admin.categories;

/**
 * @author Aleksandr Khalitov
 */
public class Category {

  private String name;

  private Level level;

  public String getName() {
    return name;
  }

  public Category setName(String name) {
    this.name = name;
    return this;
  }

  public Level getLevel() {
    return level;
  }

  public Category setLevel(Level level) {
    this.level = level;
    return this;
  }

  public static enum Level {
    DEBUG, INFO, WARN, ERROR, FATAL
  }
}
