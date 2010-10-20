package mobi.eyeline.informer.admin.delivery;

/**
 * Поля сообщения
 * @author Aleksandr Khalitov
 */
public enum MessageFields {
  STATE,
  DATE,
  ABONENT,
  TEXT,
  ERROR_CODE,
  USER_DATE;
  public static MessageFields[] ALL = MessageFields.values();

}
