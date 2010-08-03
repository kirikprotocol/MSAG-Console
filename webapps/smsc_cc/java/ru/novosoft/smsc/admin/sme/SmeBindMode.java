package ru.novosoft.smsc.admin.sme;

/**
 * Возможные режимы подключения SME к СМСЦ
 * @author Artem Snopkov
 */
public enum SmeBindMode {
  /**
   * Трансмитер
   */
  TX,
  /**
   * Ресивер
   */
  RX,
  /**
   * Трансивер
   */
  TRX
}
