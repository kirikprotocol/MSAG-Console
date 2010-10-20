package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Фильтр описывающий ограничения на записи статистики
 * @author Artem Snopkov
 */
public class DeliveryStatFilter {

  private Date fromDate;
  private Date tillDate;
  private String user;
  private Integer taskId;

  public DeliveryStatFilter() {
  }

  public DeliveryStatFilter(Date fromDate, Date tillDate, String user, Integer taskId) {
    this.fromDate = fromDate;
    this.tillDate = tillDate;
    this.user = user;
    this.taskId = taskId;
  }

  /**
   * Возвращает нижнее ограничение на дату записи с точностью до минуты или null, если ограничение отсутствует.
   * @return нижнее ограничение на дату записи с точностью до минуты или null, если ограничение отсутствует.
   */
  public Date getFromDate() {
    return fromDate;
  }
  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  /**
   * Возвращает верхнее ограничение на дату записи с точностью до минуты или null, если ограничение отсутствует.
   * @return верхнее ограничение на дату записи с точностью до минуты или null, если ограничение отсутствует.
   */
  public Date getTillDate() {
    return tillDate;
  }
  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  /**
   * Возвращает пользователя, чьи рассылки необходимо перебрать или null, если ограничение отсутствует.
   * @return пользователя, чьи рассылки необходимо перебрать или null, если ограничение отсутствует.
   */
  public String getUser() {
    return user;
  }
  public void setUser(String user) {
    this.user = user;
  }


  /**
   * Возвращает идентификатор рассылки, чьи записи надо перебрать или null, если ограничение отсутствует.
   * @return идентификатор рассылки, чьи записи надо перебрать или null, если ограничение отсутствует.
   */
  public Integer getTaskId() {
    return taskId;
  }
  public void setTaskId(Integer taskId) {
    this.taskId = taskId;
  }





}
