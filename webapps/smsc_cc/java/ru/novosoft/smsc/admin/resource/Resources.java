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
    this.strings = new HashMap<String, String>(copy.strings);
    this.childs = new HashMap<String, Resources>();
    for (Map.Entry<String, Resources> e : copy.childs.entrySet())
      this.childs.put(e.getKey(), new Resources(e.getValue()));
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
