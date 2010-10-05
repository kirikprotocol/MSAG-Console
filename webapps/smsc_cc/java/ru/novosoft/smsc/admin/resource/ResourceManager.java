package ru.novosoft.smsc.admin.resource;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public interface ResourceManager extends SmscConfiguration {

  /**
   * Возвращает список локалей, для которых существуют ресурсы
   *
   * @return список локалей, для которых существуют ресурсы
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  Collection<String> getLocales() throws AdminException;

  /**
   * Проверяет. есть ли ресурсы для указанной локали
   *
   * @param locale локаль
   * @return true, если в менеджере есть ресурсы для локали
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  boolean containsLocale(String locale) throws AdminException;

  /**
   * Добавляет или обновляет настройки ресурсов для локали
   *
   * @param locale    локаль
   * @param resources ресурсы
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  void addResourceSettings(String locale, ResourceSettings resources) throws AdminException;

  /**
   * Удаляет настройки ресурсов для заданной локали
   *
   * @param locale локаль
   * @return true, если локаль известна
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  boolean removeResourceSettings(String locale) throws AdminException;

  /**
   * Возвращает настройки ресурсов для заданной локали
   *
   * @param locale локаль
   * @return ресурсы для заданной локали или null, если локаль не известна
   * @throws ru.novosoft.smsc.admin.AdminException если произошла ошибка
   */
  ResourceSettings getResourceSettings(String locale) throws AdminException;
}
