package ru.novosoft.smsc.admin.smsc_config;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Слушатель конфига смсц
 *
 * @author Aleksandr Khalitov
 */
public interface SmscConfigListener {

  /**
   * Вызывается перед изменением общих настроек смсц
   *
   * @param cs новые настройки
   * @throws AdminException прерывает изменение настроек
   */
  public void setCommonSettings(CommonSettings cs) throws AdminException;

  /**
   * Вызывается перед изменением настроек инстанца смсц
   *
   * @param is новые настройки
   * @throws AdminException прерывает изменение настроек
   */
  public void setInstanceSettings(InstanceSettings is) throws AdminException;

  /**
   * Вызывается перед cбросом настроек
   *
   * @param cs новые общие настройки
   * @param is новые настройки инстанцов
   * @throws AdminException прерывает изменение настроек
   */
  public void resetSettings(CommonSettings cs, InstanceSettings[] is) throws AdminException;

  /**
   * Вызывается перед сохранением настроек
   *
   * @param cs новые общие настройки
   * @param is новые настройки инстанцов
   * @throws AdminException прерывает изменение настроек
   */
  public void applySettings(CommonSettings cs, InstanceSettings[] is) throws AdminException;


}
