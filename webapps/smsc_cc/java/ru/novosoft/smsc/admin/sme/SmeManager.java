package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public interface SmeManager extends SmscConfiguration {
  /**
   * Добавляет новую SME в список или обновляет данные об SME
   *
   * @param smeId  идентификатор SME
   * @param newSme настройки SME
   * @throws ru.novosoft.smsc.admin.AdminException еслипроизошла ошибка
   */
  void addSme(String smeId, Sme newSme) throws AdminException;

  /**
   * Возвращает настройки SME по её идентификатору
   *
   * @param smeId идентификатор SME
   * @return Настройки SME или null, если SME с заданным идентификатором не существует.
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  Sme getSme(String smeId) throws AdminException;

  /**
   * Удаляет SME из списка по её идентификатору.
   *
   * @param smeId идентификатор SME.
   * @return true, если SME с таким идентификатором существует.
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка.
   */
  boolean removeSme(String smeId) throws AdminException;

  /**
   * Проверяет, существует ли SME с указанным идентификатором
   *
   * @param smeId идентификатор SME
   * @return true, если SME с указанным идентификатором присутствует в списке.
   */
  boolean contains(String smeId);

  /**
   * Возвращает информацию обо всех SME
   *
   * @return список настроек всех SME
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  Map<String, Sme> smes() throws AdminException;

  /**
   * Возвращает статусы всех зарегистрированных SME с точки зрения СМС центров
   *
   * @return статус SME в СМСЦ или null, если SME с таким идентификатором не существует
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  Map<String, SmeSmscStatuses> getSmesSmscStatuses() throws AdminException;

  /**
   * Отключает SME с указанными идентификаторами от всех инстанцев SMSC при помощи SMPP UNBIND
   *
   * @param smeIds идентификаторы SME-х
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void disconnectSmeFromSmsc(String... smeIds) throws AdminException;

  /**
   * Отключает SME с указанными идентификаторами от всех инстанцев SMSC при помощи SMPP UNBIND
   *
   * @param smeIds идентификаторы SME-х
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void disconnectSmeFromSmsc(List<String> smeIds) throws AdminException;

  /**
   * Возвращает статус SME с точки зрения демона.
   *
   * @param smeId идентификатор SME
   * @return статус SME в демоне или null, если SME не зарегистрирована в демоне
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  SmeServiceStatus getSmeServiceStatus(String smeId) throws AdminException;

  /**
   * Запускает SME на дефолтном хосте
   *
   * @param smeId идентификатор SME
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void startSme(String smeId) throws AdminException;

  /**
   * Останавливает SME
   *
   * @param smeId идентификатор SME
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void stopSme(String smeId) throws AdminException;

  /**
   * Переключает SME на указанный хост. Список хостов, на которых может быть запущена SME,
   * можно узнать из SmeServiceStatus.
   *
   * @param smeId  идентификатор SME
   * @param toHost хост, на который надо переключить SME
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void switchSme(String smeId, String toHost) throws AdminException;
}
