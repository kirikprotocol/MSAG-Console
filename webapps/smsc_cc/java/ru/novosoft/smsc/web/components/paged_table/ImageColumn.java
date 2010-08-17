package ru.novosoft.smsc.web.components.paged_table;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class ImageColumn extends Column {

  private Map<String, String> valueMapping;

  public ImageColumn(String name, String title) {
    super(name, title);
  }

  public Map<String, String> getValueMapping() {
    return valueMapping;
  }

  public void setValueMapping(Map<String, String> valueMapping) {
    this.valueMapping = valueMapping;
  }
}
