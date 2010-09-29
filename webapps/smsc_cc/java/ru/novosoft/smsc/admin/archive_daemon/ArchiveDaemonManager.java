package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public interface ArchiveDaemonManager {

  /**
   * Возвращает текущие настройки ArchiveDaemon-а
   *
   * @return текущие настройки ArchiveDaemon-а
   * @throws ru.novosoft.smsc.admin.AdminException, если произошла ошибка
   */
  public ArchiveDaemonSettings getSettings() throws AdminException;

  /**
   * Обновляет настройки ArchiveDaemon-а.
   *
   * @param settings новый настройки ArchiveDaemon-а
   * @throws AdminException, если произошла ошибка
   */
  public void updateSettings(ArchiveDaemonSettings settings) throws AdminException;

  /**
   * Возвращает хост, на котором запущен демон или null, если демон offline
   * @return хост, на котором запущен демон или null, если демон offline
   * @throws AdminException, если произошла ошибка
   */
  public String getDaemonOnlineHost() throws AdminException;

  /**
   * Переключает демона на указанную ноду
   * @param toHost хост, на который надо переключить демона.
   * @throws AdminException, если произошла ошибка
   */
  public void switchDaemon(String toHost) throws AdminException;

  /**
   * Запускает демона
   * @throws AdminException если произошла ошибка
   */
  public void startDaemon() throws AdminException;

  /**
   * Останавливает демона
   * @throws AdminException если произошла ошибка
   */
  public void stopDaemon() throws AdminException;

  /**
   * Водвращает список хостов, на которых может быть запущен демон
   * @return список хостов, на которых может быть запущен демон
   * @throws AdminException если произошла ошибка
   */
  public List<String> getDaemonHosts() throws AdminException;
}
