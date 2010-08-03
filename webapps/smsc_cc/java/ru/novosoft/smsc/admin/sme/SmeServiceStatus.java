package ru.novosoft.smsc.admin.sme;

import java.util.List;

/**
 * Статус SME в демоне
 * @author Artem Snopkov
 */
public class SmeServiceStatus {

  private final String onlineHost;
  private final List<String> hosts;

  SmeServiceStatus(String onlineHost, List<String> hosts) {
    this.onlineHost = onlineHost;
    this.hosts = hosts;
  }

  /**
   * Возвращает название хоста, на котором запущена SME или null, если SME в offline
   * @return название хоста, на котором запущена SME или null, если SME в offline
   */
  public String getOnlineHost() {
    return onlineHost;
  }

  /**
   * Возвращает список хостов, на которых может быть запущена SME
   * @return список хостов, на которых может быть запущена SME
   */
  public List<String> getHosts() {
    return hosts;
  }
}
