package mobi.eyeline.informer.admin.monitoring;



import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class MonitoringFilter {
  
  
  private Date startDate;

  private Date endDate;

  private MBean.Source source;


  public MBean.Source getSource() {
    return source;
  }

  public void setSource(MBean.Source source) {
    this.source = source;
  }

  public Date getStartDate() {
    return startDate;
  }

  public MonitoringFilter setStartDate(Date startDate) {
    this.startDate = startDate;
    return this;
  }

  public Date getEndDate() {
    return endDate;
  }

  public MonitoringFilter setEndDate(Date endDate) {
    this.endDate = endDate;
    return this;
  }

  /**
   * Показывать запись или нет
   *
   * @param event запись
   * @return true - показывать запись, false - нет
   */
  @SuppressWarnings({"RedundantIfStatement"})
  boolean accept(MonitoringEvent event) {
    if (source != null && !source.equals(event.getSource())) {
      return false;
    }
    if (startDate != null && startDate.getTime() > event.getTime()) {
      return false;
    }
    if (endDate != null && endDate.getTime() < event.getTime()) {
      return false;
    }
    return true;
  }
}
