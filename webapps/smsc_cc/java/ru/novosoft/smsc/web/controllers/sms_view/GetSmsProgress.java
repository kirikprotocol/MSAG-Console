package ru.novosoft.smsc.web.controllers.sms_view;

/**
 * @author Artem Snopkov
 */
public class GetSmsProgress {

  private int total;
  private int current;

  public int getTotal() {
    return total;
  }

  public void setTotal(int total) {
    this.total = total;
  }

  public int getCurrent() {
    return current;
  }

  public void setCurrent(int current) {
    this.current = current;
  }
}
