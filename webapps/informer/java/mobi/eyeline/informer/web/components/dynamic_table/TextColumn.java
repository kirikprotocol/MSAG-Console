package mobi.eyeline.informer.web.components.dynamic_table;


/**
 * @author Artem Snopkov
 */
class TextColumn extends Column {

  boolean allowEditAfterAdd = true;
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
}
