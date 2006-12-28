package ru.novosoft.smsc.jsp.util;

import java.util.HashMap;

/**
 * User: artem
 * Date: 27.12.2006
 */
public class BeanTemporaryContent {
  private final String contentName;
  private final HashMap attributes = new HashMap();

  public BeanTemporaryContent(String contentName) {
    this.contentName = contentName;
  }

  String getName() {
    return contentName;
  }

  void addAttribute(Object key, Object value) {
    attributes.put(key, value);
  }

  Object getAttribute(Object key) {
    return attributes.get(key);
  }

}
