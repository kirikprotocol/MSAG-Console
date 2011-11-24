package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public interface MCISmeManager {

  /**
   * Возвращает текущие настройки MCISme
   *
   * @return текущие настройки MCISme
   * @throws AdminException если произошла ошибка
   */
  public MCISmeSettings getSettings() throws AdminException;

  /**
   * Обновляет настройки MCISme.
   *
   * @param settings новый настройки MCISme
   * @throws AdminException, если произошла ошибка
   */
  public void updateSettings(MCISmeSettings settings) throws AdminException;

  /**
   * Статистика сообщений
   * @param filter фильтр
   * @return статистика сообщений
   * @throws AdminException ошибка
   */
  public MessageStatistics getStatistics(StatFilter filter) throws AdminException;

  /**
   * Возвращает хост, на котором запущен сервис или null, если сервис offline
   *
   * @return хост, на котором запущен сервис или null, если сервис offline
   * @throws AdminException, если произошла ошибка
   */

  public String getSmeOnlineHost() throws AdminException;

  /**
   * Переключает сервис на указанную ноду
   *
   * @param toHost хост, на который надо переключить сервис.
   * @throws AdminException, если произошла ошибка
   */
  public void switchSme(String toHost) throws AdminException;

  /**
   * Запускает сервис
   *
   * @throws AdminException если произошла ошибка
   */
  public void startSme() throws AdminException;

  /**
   * Останавливает сервис
   *
   * @throws AdminException если произошла ошибка
   */
  public void stopSme() throws AdminException;

  /**
   * Водвращает список хостов, на которых может быть запущен сервис
   *
   * @return список хостов, на которых может быть запущен сервис
   * @throws AdminException если произошла ошибка
   */
  public List<String> getSmeHosts() throws AdminException;

}
