package ru.novosoft.smsc.admin.sme;

/**
 * Возможные способы подключения SME
 * @author Artem Snopkov
 */
public enum SmeConnectType {
  /**
   * SME подключена к Load balancer
   */
  VIA_LOAD_BALANCER,
  /**
   * SME подключена напрямую к центрам
   */
  DIRECT_CONNECT
}
