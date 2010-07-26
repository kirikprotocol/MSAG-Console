package ru.novosoft.smsc.admin.cluster_controller;

import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class ConfigState {
  private long ccLastUpdateTime;
  private Map<Integer, Long> instancesUpdateTimes;

  ConfigState(long ccLastUpdateTime, Map<Integer, Long> instancesUpdateTimes) {
    this.ccLastUpdateTime = ccLastUpdateTime;
    this.instancesUpdateTimes = instancesUpdateTimes;
  }

  public long getCcLastUpdateTime() {
    return ccLastUpdateTime;
  }

  public Map<Integer, Long> getInstancesUpdateTimes() {
    return instancesUpdateTimes;
  }
}
