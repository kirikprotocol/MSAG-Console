package ru.novosoft.smsc.admin.smsc_config;

/**
 * Класс, управляющий конфигурационным файлом СМСЦ. Использует SmscConfig для чтения/записи конфига.
 *
 * @author Artem Snopkov
 */
public class SmscConfigManager {

  /**
   * Возвращает настройки, общие для всех СМСЦ.
   * @return настройки, общие для всех СМСЦ.
   */
  public CommonSettings getCommonSettings() {
    return null;
  }

  /**
   * Возвращает количество инстанцев СМСЦ
   * @return количество инстанцев СМСЦ
   */
  public int getSmscInstancesCount() {
    return 0;
  }

  /**
   * Возвращает настройки, специфические для каждого инстанца СМСЦ
   * @return настройки, специфические для каждого инстанца СМСЦ
   */
  public InstanceSettings getInstanceSettings(int instanceNumber) {
    return null;
  }

  /**
   * Сохраняет и применяет изменения, сделанные в конфиге
   */
  public void apply() {

  }

  /**
   * Откатывает изменения, сделанные в конфиге
   */
  public void reset() {

  }

}
