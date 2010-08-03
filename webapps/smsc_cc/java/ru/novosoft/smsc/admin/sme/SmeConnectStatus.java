package ru.novosoft.smsc.admin.sme;

/**
 * Состояния соединения между СМСЦ и SME
 * @author Artem Snopkov
 */
public enum SmeConnectStatus {
  /**
   * SME прибиндена к центру
   */
  CONNECTED,
  /**
   * SME отключена от центра
   */
  DISCONNECTED,
  /**
   * SME является встроенной в СМСЦ. Такая SME запускается вместе с центром,
   * всегда приконнекцена к нему и не может быть отключена от центра
   */
  INTERNAL
}
