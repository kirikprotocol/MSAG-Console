package mobi.eyeline.informer.admin.delivery;

/**
 * Поля рассылки
 *
 * @author Aleksandr Khalitov
 */
public enum DeliveryFields {
  UserId,
  Name,
  Status,
  StartDate,
  EndDate,
  ActivityPeriod;
  public static DeliveryFields[] ALL = DeliveryFields.values();
}
