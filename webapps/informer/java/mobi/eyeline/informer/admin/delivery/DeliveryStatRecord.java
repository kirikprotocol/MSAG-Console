package mobi.eyeline.informer.admin.delivery;

/**
 * Запись в файле со статистикой
 * @author Artem Snopkov
 */
public class DeliveryStatRecord {
  private int year;
  private int month;
  private int day;
  private int hour;
  private int minute;
  private int taskId;
  private int delivered;
  private int failed;
  private String user;

  public DeliveryStatRecord(String user, int year, int month, int day, int hour, int minute, int taskId, int delivered, int failed) {
    this.user=user;
    this.year = year;
    this.month = month;
    this.day = day;
    this.hour = hour;
    this.minute = minute;
    this.taskId = taskId;
    this.delivered = delivered;
    this.failed = failed;
  }

  /**
   * Возвращает год, к которому относится запись
   * @return год, к которому относится запись
   */
  public int getYear() {
    return year;
  }

  /**
   * Возвращает месяц (1-12), к которому относится запись
   * @return месяц, к которому относится запись
   */
  public int getMonth() {
    return month;
  }

  /**
   * Возвращает день (1-31), к которому относится запись
   * @return день, к которому относится запись
   */
  public int getDay() {
    return day;
  }

  /**
   * Возвращает час (0-23), к которому относится запись
   * @return
   */
  public int getHour() {
    return hour;
  }

  /**
   * Возвращает минуту (0-59), к которой относится запись
   * @return минуту, к которой относится запись
   */
  public int getMinute() {
    return minute;
  }

  /**
   * Возвращает идентификатор рассылки, к которой относится запись
   * @return идентификатор рассылки, к которой относится запись
   */
  public int getTaskId() {
    return taskId;
  }

  /**
   * Возвращает количество сообщений из рассылки, доставленный в указанную минуту
   * @return количество сообщений из рассылки, доставленный в указанную минуту
   */
  public int getDelivered() {
    return delivered;
  }

  /**
   * Возвращает количество сообщений из рассылки, доставка которых провалилась в указанную минуту
   * @return количество сообщений из рассылки, доставка которых провалилась в указанную минуту
   */
  public int getFailed() {
    return failed;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }
}
