package mobi.eyeline.informer.admin.delivery;

/**
 * Поля рассылки
 * @author Aleksandr Khalitov
 */
public enum DeliveryFields {
  USER_ID,
  NAME,
  STATUS,
  STARDATE,
  ENDDATE,
  ACTIVITY_PERIOD;
  
  public static DeliveryFields[] ALL = DeliveryFields.values();
}
