package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Запись в файле со статистикой
 * @author Artem Snopkov
 */
public class DeliveryStatRecord {
  private Date date;

  private int taskId;
  private int delivered;
  private int failed;
  private String user;

  public DeliveryStatRecord(String user, Date date, int taskId, int delivered, int failed) {
    this.user=user;
    this.date = date;
    this.taskId = taskId;
    this.delivered = delivered;
    this.failed = failed;
  }

  /**
   * Возвращает дату ( округленную до минуты)   к которой относится запись
   * @return  дату ( округленную до минуты)   к которой относится запись
   */

  public Date getDate() {
    return date;
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
