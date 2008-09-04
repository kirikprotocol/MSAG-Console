package ru.novosoft.smsc.emailsme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Date;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class StatisticsFilter implements Filter {

  private Date from = new Date();
  private Date till = new Date();

  public StatisticsFilter(Date from, Date till) {
    this.from = from;
    this.till = till;
  }

  public StatisticsFilter() {
  }

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

  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    StatisticDataItem i = (StatisticDataItem)item;
    Date d = (Date)i.getValue("date");
    return !d.before(from) && !d.after(till);
  }
}
