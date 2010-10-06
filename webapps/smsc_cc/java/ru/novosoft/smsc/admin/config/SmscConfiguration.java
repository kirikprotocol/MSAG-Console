package ru.novosoft.smsc.admin.config;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Map;

/**
 * Интерфейс описывает конфигурацию, относящуюся к СМСЦ.
 * @author Artem Snopkov
 */
public interface SmscConfiguration {

  /**
   * Возвращает состояние конфигурации для всех запущенных инстанцев СМСЦ. Ключем является номер инстанца,
   * значением - статус конфига. Инстанцы СМСЦ, которые в данный момент остановлены в результате не учитываются.
   * @return состояние конфигурации для всех запущенных инстанцев СМСЦ или null, если Cluster Controller отключен
   */
  Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException;
}
