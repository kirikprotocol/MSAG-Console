package mobi.eyeline.informer.admin.notifications;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:43:59
 */
public interface DeliveryNotificationsListener {

  //todo Почему бы не сделать по 1 методу на каждый тип нотификации?
  // todo Данная реализация обрекает клиентов на проверку типа нотификации в каждом листенере.
  public void onDeliveryNotification(DeliveryNotification notification);

}
