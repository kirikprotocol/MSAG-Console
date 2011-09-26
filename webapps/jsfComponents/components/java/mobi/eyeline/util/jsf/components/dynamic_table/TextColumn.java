package mobi.eyeline.util.jsf.components.dynamic_table;


/**
 * @author Artem Snopkov
 */
class TextColumn extends Column {

  boolean allowEditAfterAdd = true;
  boolean allowEmpty = false;
  int maxLength;

  public boolean isAllowEditAfterAdd() {
    return allowEditAfterAdd;
  }

  public void setAllowEditAfterAdd(boolean allowEditAfterAdd) {
    this.allowEditAfterAdd = allowEditAfterAdd;
  }

  public int getMaxLength() {
    return maxLength;
  }

  public void setMaxLength(int maxLength) {
    this.maxLength = maxLength;
  }

  public String getSize() {
    return null;
  }

  public boolean isAllowEmpty() {
    return allowEmpty;
  }

  public void setAllowEmpty(boolean allowEmpty) {
    this.allowEmpty = allowEmpty;
  }
}
