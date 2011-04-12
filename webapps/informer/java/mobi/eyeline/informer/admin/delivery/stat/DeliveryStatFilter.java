package mobi.eyeline.informer.admin.delivery.stat;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * Фильтр описывающий ограничения на записи статистики
 *
 * @author Artem Snopkov
 */
public class DeliveryStatFilter {

  private Date fromDate;
  private Date tillDate;
  private String user;
  private List<Integer> taskIds;

  private Integer regionId;

  public DeliveryStatFilter() {
  }

  public DeliveryStatFilter(Date fromDate, Date tillDate, String user, List<Integer> taskIds) {
    this.fromDate = fromDate;
    this.tillDate = tillDate;
    this.user = user;
    this.taskIds = taskIds;
  }

  public DeliveryStatFilter(DeliveryStatFilter other) {
    this.fromDate = other.fromDate == null ? null : (Date) other.fromDate.clone();
    this.tillDate = other.tillDate == null ? null : (Date) other.tillDate.clone();
    this.user = other.user;
    this.taskIds = other.taskIds==null ? null : new ArrayList<Integer>(other.taskIds);
    this.regionId = other.regionId;
  }

  /**
   * Возвращает нижнее ограничение на дату записи с точностью до минуты или null, если ограничение отсутствует.
   *
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
   *
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
   *
   * @return пользователя, чьи рассылки необходимо перебрать или null, если ограничение отсутствует.
   */
  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    if (user != null && user.trim().length() == 0) user = null;
    this.user = user;
  }


  /**
   * Возвращает идентификатор рассылки, чьи записи надо перебрать или null, если ограничение отсутствует.
   *
   * @return идентификатор рассылки, чьи записи надо перебрать или null, если ограничение отсутствует.
   */
  public List<Integer> getTaskIds() {
    return taskIds;
  }

  public void setTaskIds(List<Integer> taskId) {
    this.taskIds = taskId;
  }

  public Integer getRegionId() {
    return regionId;
  }

  public void setRegionId(Integer regionId) {
    this.regionId = regionId;
  }
}
