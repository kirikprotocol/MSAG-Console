package ru.novosoft.smsc.admin.service;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Структура, содержащая описание сервиса
 */
public class ServiceInfo {

  private final String serviceId;
  private final List<String> hosts;
  private final String onlineHost;
  private final File baseDir;

  public ServiceInfo(String serviceId, List<String> hosts, String onlineHost, File baseDir) {
    this.serviceId = serviceId;
    this.hosts = hosts;
    this.onlineHost = onlineHost;
    this.baseDir = baseDir;
  }

  public ServiceInfo(String serviceId, String[] hosts, String onlineHost, File baseDir) {
    this.serviceId = serviceId;
    this.hosts = new ArrayList<String>(hosts.length);
    for (String host : hosts)
      this.hosts.add(host);
    this.onlineHost = onlineHost;
    this.baseDir = baseDir;
  }

  /**
   * Возвращает идентификатор сервиса
   * @return идентификатор сервиса
   */
  public String getServiceId() {
    return serviceId;
  }

  /**
   * Возвращает список хостов, на которых может быть запущен сервис
   * @return список хостов, на которых может быть запущен сервис
   */
  public List<String> getHosts() {
    return hosts;
  }

  /**
   * Возвращает название хоста, на котором запущен сервис или null, если сервис остановлен.
   * @return название хоста, на котором запущен сервис или null, если сервис остановлен.
   */
  public String getOnlineHost() {
    return onlineHost;
  }

  /**
   * Возвращает путь к базовой директории сервиса. В этой директории назодятся бинарники и конфиги сервиса
   * @return путь к базовой директории сервиса.
   */
  public File getBaseDir() {
    return baseDir;
  }
}
