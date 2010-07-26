package ru.novosoft.smsc.admin.cluster_controller;

import java.util.Map;

/**
 * Структура, описывающая статус конфигурации
 * @author Artem Snopkov
 */
public class ConfigState {
  private long ccLastUpdateTime;
  private Map<Integer, Long> instancesUpdateTimes;

  public ConfigState(long ccLastUpdateTime, Map<Integer, Long> instancesUpdateTimes) {
    this.ccLastUpdateTime = ccLastUpdateTime;
    this.instancesUpdateTimes = instancesUpdateTimes;
  }

  /**
   * Возвращает время последнего обновления конфигурации в Cluster Controller-е
   * @return время последнего обновления конфигурации в Cluster Controller-е
   */
  public long getCcLastUpdateTime() {
    return ccLastUpdateTime;
  }

  /**
   * Возвращает Map, где ключем является номер инстанца СМСЦ, а значением - время
   * последнего обновления конфигурации этим СМСЦ.
   * @return Map, где ключем является номер инстанца СМСЦ, а значением - время
   * последнего обновления конфигурации этим СМСЦ.
   */
  public Map<Integer, Long> getInstancesUpdateTimes() {
    return instancesUpdateTimes;
  }
}
