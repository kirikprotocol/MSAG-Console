package ru.novosoft.smsc.web.config;

import ru.novosoft.smsc.web.auth.User;

/**
 * @author Artem Snopkov
 */
public class UpdateInfo {
  
  private long lastUpdateTime;
  private String lastUpdateString;
  private boolean changed;

  UpdateInfo(long lastUpdateTime, String lastUpdateString, boolean changed) {
    this.lastUpdateTime = lastUpdateTime;
    this.lastUpdateString = lastUpdateString;
    this.changed = changed;
  }

  public long getLastUpdateTime() {
    return lastUpdateTime;
  }

  public String getLastUpdateString() {
    return lastUpdateString;
  }

  public boolean isChanged() {
    return changed;
  }
}
