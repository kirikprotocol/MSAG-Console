package ru.novosoft.smsc.admin.sme;

/**
 * Структура, описывающая состояние SME для всех инстанцев СМСЦ
 *
 * @author Artem Snopkov
 */
public class SmeSmscStatuses {

  private final SmeConnectType connectType;
  private final SmeSmscStatus statuses[];

  SmeSmscStatuses(SmeConnectType connectType, SmeSmscStatus[] statuses) {
    this.connectType = connectType;
    this.statuses = statuses;
  }

  /**
   * Возвращает способ подключения SME к центрам
   *
   * @return способ подключения SME к центрам
   */
  public SmeConnectType getConnectType() {
    return connectType;
  }

  /**
   * Возвращает массив, в каждом элементе которого описан статус подключения SME к
   * конкретному активному инстанцу центра. Если в данный момент инстанц центра опущен, то в данном
   * массиве статус SME для данного инстанца будет отсутствовать.
   *
   * @return статусы SME во всех активных инстанцах СМСЦ
   */
  public SmeSmscStatus[] getStatuses() {
    return statuses;
  }
}
