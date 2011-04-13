package ru.sibinco.smsx.stats.backend;

import java.util.Date;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRequestPrototype {

  private Date from;

  private Date till;

  private Set reportTypeFilter;

  private Set serviceIdFilter;

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public Set getReportTypeFilter() {
    return reportTypeFilter;
  }

  public void setReportTypeFilter(Set reportTypeFilter) {
    this.reportTypeFilter = reportTypeFilter;
  }

  public Set getServiceIdFilter() {
    return serviceIdFilter;
  }

  public void setServiceIdFilter(Set serviceIdFilter) {
    this.serviceIdFilter = serviceIdFilter;
  }
}
