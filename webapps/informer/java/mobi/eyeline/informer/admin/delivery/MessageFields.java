package mobi.eyeline.informer.admin.delivery;

/**
 * Поля сообщения
 *
 * @author Aleksandr Khalitov
 */
public enum MessageFields {
  State,
  Date,
  Abonent,
  Text,
  ErrorCode;
  public static MessageFields[] ALL = MessageFields.values();

}
