package ru.novosoft.smsc.web.components.dynamic_table.model;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class DynamicTableRow implements Serializable {

  Map<String, Object> values = new HashMap<String, Object>();

  public void setValue(String columnName, Object value) {
    values.put(columnName, value);
  }

  public Object getValue(String columnName) {
    return values.get(columnName);
  }
}
