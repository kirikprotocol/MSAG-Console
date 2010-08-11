package ru.novosoft.smsc.web.components.dynamic_table;

import ru.novosoft.smsc.web.components.Validation;

import java.util.List;

/**
 * @author Artem Snopkov
 */
class SelectColumn extends Column {

  private boolean allowEditAfterAdd = true;
  private List values;
  private boolean uniqueValues;

  public void setAllowEditAfterAdd(boolean allowEditAfterAdd) {
    this.allowEditAfterAdd = allowEditAfterAdd;
  }

  public void setValues(List<String> values) {
    this.values = values;
  }

  public List getValues() {
    return values;
  }

  public boolean isAllowEditAfterAdd() {
    return allowEditAfterAdd;
  }

  public Validation getValidation() {
    return null;
  }

  public boolean isUniqueValues() {
    return uniqueValues;
  }

  public void setUniqueValues(boolean uniqueValues) {
    this.uniqueValues = uniqueValues;
  }
}
