package ru.novosoft.smsc.admin.resource;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class Resources {

  private Map<String, String> strings = new HashMap<String, String>();
  private Map<String, Resources> childs = new HashMap<String, Resources>();

  public Resources() {

  }

  Resources(Resources copy) {
    setStrings(copy.strings);
    setChilds(copy.childs);
  }

  public Map<String, String> getStrings() {
    return strings;
  }

  public String getString(String key) {
    return strings.get(key);
  }

  public void setStrings(Map<String, String> strings) {
    this.strings = strings;
  }

  public Map<String, Resources> getChilds() {
    return Collections.unmodifiableMap(childs);
  }

  public Resources getChild(String name) {
    return childs.get(name);
  }

  public void setChilds(Map<String, Resources> childs) {
    this.childs = childs;
  }
  
}
