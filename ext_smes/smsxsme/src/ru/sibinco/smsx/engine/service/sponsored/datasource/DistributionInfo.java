package ru.sibinco.smsx.engine.service.sponsored.datasource;

import java.util.Date;


/**
 * User: artem
 * Date: 29.06.2007
 */

public class DistributionInfo {

  private String abonent;
  private int todayCount;
  private long currentCount;
  private int cnt;
  private boolean exists = false;


  public String getAbonent() {
    return abonent;
  }

  public void setAbonent(String abonent) {
    this.abonent = abonent;
  }

  public int getTodayCount() {
    return todayCount;
  }

  public void setTodayCount(int todayCount) {
    this.todayCount = todayCount;
  }

  public long getCurrentCount() {
    return currentCount;
  }

  public void setCurrentCount(long currentCount) {
    this.currentCount = currentCount;
  }

  public int getCnt() {
    return cnt;
  }

  public void setCnt(int cnt) {
    this.cnt = cnt;
  }

  public boolean isExists() {
    return exists;
  }

  void setExists(boolean exists) {
    this.exists = exists;
  }
}
