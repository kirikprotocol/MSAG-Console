package ru.novosoft.smsc.jsp.util.tables.impl.blacknick;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNickQuery {

  private int expectedResultsQuantity;
  private String filter;
  private String sortOrder;

  public BlackNickQuery(int expectedResultsQuantity, String filter, String sortOrder) {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = filter;
    this.sortOrder = sortOrder;
  }

  public int getExpectedResultsQuantity() {
    return expectedResultsQuantity;
  }

  public void setExpectedResultsQuantity(int expectedResultsQuantity) {
    this.expectedResultsQuantity = expectedResultsQuantity;
  }

  public String getFilter() {
    return filter;
  }

  public void setFilter(String filter) {
    this.filter = filter;
  }

  public String getSortOrder() {
    return sortOrder;
  }

  public void setSortOrder(String sortOrder) {
    this.sortOrder = sortOrder;
  }
}
