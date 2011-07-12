package mobi.eyeline.informer.admin.delivery;

/**
 * Состояние сообщения
 *
 * @author Aleksandr Khalitov
 */
public enum MessageState {
  New,
  Process,
  Delivered,
  Failed,
  Expired,
  Sent,
  Retry
}
