package ru.novosoft.smsc.admin.sme;

import java.io.Serializable;

/**
 * Статус SME в СМСЦ
 *
 * @author Artem Snopkov
 */
public class SmeSmscStatus implements Serializable {

  private final int smscInstanceNumber;
  private final SmeConnectStatus connectionStatus; // Internal или Disconnected
  private final SmeBindMode bindMode;
  private final String peerIn;
  private final String peerOut;

  SmeSmscStatus(int smscInstanceNumber, SmeConnectStatus connectionStatus, SmeBindMode bindMode, String peerIn, String peerOut) {
    this.smscInstanceNumber = smscInstanceNumber;
    this.connectionStatus = connectionStatus;
    this.bindMode = bindMode;
    this.peerIn = peerIn;
    this.peerOut = peerOut;
  }

  /**
   * Возвращает номер инстанца СМСЦ
   *
   * @return номер инстанца СМСЦ
   */
  public int getSmscInstanceNumber() {
    return smscInstanceNumber;
  }

  /**
   * Возвращает статус подключения SME к данному СМСЦ или null, если статус неизвестен
   *
   * @return статус подключения SME к данному СМСЦ или null, если статус неизвестен
   */
  public SmeConnectStatus getConnectionStatus() {
    return connectionStatus;
  }

  /**
   * Возвращает режим подключения SME к данному СМСЦ или null, если режим неизвестен
   *
   * @return режим подключения SME к данному СМСЦ или null, если режим неизвестен
   */
  public SmeBindMode getBindMode() {
    return bindMode;
  }

  /**
   * Возвращает хост, на котором запущен ресивер
   *
   * @return хост, на котором запущен ресивер
   */
  public String getPeerIn() {
    return peerIn;
  }

  /**
   * Возвращает хост, на котором запущен трансмитер.
   *
   * @return хост, на котором запущен трансмитер.
   */
  public String getPeerOut() {
    return peerOut;
  }
}
