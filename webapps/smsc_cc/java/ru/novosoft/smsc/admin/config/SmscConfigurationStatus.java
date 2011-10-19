package ru.novosoft.smsc.admin.config;

/**
 * Перечисление всех возможных состояний конфигурации, применительно к СМСЦ
 * @author Artem Snopkov
 */
public enum SmscConfigurationStatus {
  /**
   * СМСЦ использует актуальную конфигурацию
   */
  UP_TO_DATE,
  /**
   * СМСЦ использует устаревшую конфигурацию
   */
  OUT_OF_DATE,
  /**
   * СМСЦ не использует данную конфигурацию (при сборке СМСЦ поддожка была отключена)
   */
  NOT_SUPPORTED
}
