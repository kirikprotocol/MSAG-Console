package ru.sibinco.smpp.ub_sme;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 20.07.2007
 * Time: 13:57:28
 * To change this template use File | Settings | File Templates.
 */
public class RegionInfo {
  private String name;
  private boolean isPS;
  private String threadsPoolName;
  private String query;

  public RegionInfo(String name, boolean PS, String threadsPoolName, String query) {
    this.name = name;
    isPS = PS;
    this.threadsPoolName = threadsPoolName;
    this.query = query;
  }

  public String getName() {
    return name;
  }

  public boolean isPS() {
    return isPS;
  }

  public String getThreadsPoolName() {
    return threadsPoolName;
  }

  public String getQuery() {
    return query;
  }

}
