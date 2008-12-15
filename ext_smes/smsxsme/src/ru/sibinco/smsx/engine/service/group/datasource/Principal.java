package ru.sibinco.smsx.engine.service.group.datasource;

/**
 * User: artem
 * Date: 18.11.2008
 */
public class Principal {
  private final String address;
  private int maxLists;
  private int maxMembersPerList;

  public Principal(String address) {
    this.address = address;
  }

  public String getAddress() {
    return address;
  }

  public int getMaxLists() {
    return maxLists;
  }

  public void setMaxLists(int maxLists) {
    this.maxLists = maxLists;
  }

  public int getMaxMembersPerList() {
    return maxMembersPerList;
  }

  public void setMaxMembersPerList(int maxMembersPerList) {
    this.maxMembersPerList = maxMembersPerList;
  }
}
