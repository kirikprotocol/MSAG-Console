package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.cluster_controller.protocol.CategoryInfo;

/**
 * @author Artem Snopkov
 */
public class CCLoggingInfo {

  private final CategoryInfo info;

  public CCLoggingInfo() {
    info = new CategoryInfo("", "NOTSET");
  }

  CCLoggingInfo(CategoryInfo info) {
    this.info = info;
  }

  CCLoggingInfo(String name, Level level) {
    info = new CategoryInfo("", "NOTSET");
    setName(name);
    setLevel(level);
  }

  CategoryInfo toCategoryInfo() {
    return info;
  }

  public String getName() {
    return info.getName();
  }

  public void setName(String name) {
    info.setName(name);
  }

  public Level getLevel() {
    if (info.getLevel().equals("NOTSET"))
      return null;
    return Level.valueOf(info.getLevel());
  }

  public void setLevel(Level level) {
    if (level == null)
      info.setLevel("NOTSET");
    else
      info.setLevel(level.name());
  }

  public enum Level {
    DEBUG, INFO, WARN, ERROR, FATAL
  }
}
