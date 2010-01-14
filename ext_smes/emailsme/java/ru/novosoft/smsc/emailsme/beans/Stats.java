package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.emailsme.backend.tables.stat.StatisticsQuery;
import ru.novosoft.smsc.emailsme.backend.tables.stat.StatisticsFilter;
import ru.novosoft.smsc.emailsme.backend.tables.stat.StatisticsDataSource;
import ru.novosoft.smsc.emailsme.backend.tables.stat.StatisticDataItem;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class Stats extends SmeBean {

  private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

  private String mbQuery;
  private StatisticsFilter filter = new StatisticsFilter();
  private StatisticsDataSource ds;
  private Map stats;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      String serviceFolder = appContext.getHostsManager().getServiceInfo("emailsme").getServiceFolder().getAbsolutePath();
      String statsStoreDir = getSmeContext().getConfig().getString("stat.storeLocation");
      if( statsStoreDir.length() > 0 && statsStoreDir.charAt(0) != '/' )
        statsStoreDir = serviceFolder + '/' + statsStoreDir;
      ds = new StatisticsDataSource(statsStoreDir);
    } catch (Throwable e) {
      return error("Can't init data source", e);
    }
    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbQuery != null) {
      processQuery();
      mbQuery = null;
    }

    return RESULT_OK;
  }

  public void processQuery() {
    final StatisticsQuery query = new StatisticsQuery(filter);

    try {
      QueryResultSet rs = ds.query(query);

      stats = new HashMap();

      for (Iterator iter = rs.iterator(); iter.hasNext();) {
        StatisticDataItem item = (StatisticDataItem)iter.next();
        Calendar cal = Calendar.getInstance();
        cal.setTime(item.getDate());
        cal.set(Calendar.HOUR_OF_DAY, 0);
        cal.set(Calendar.MINUTE, 0);
        cal.set(Calendar.SECOND, 0);
        cal.set(Calendar.MILLISECOND, 0);

        Statistic s = (Statistic)stats.get(cal.getTime());
        if (s == null) {
          s = new Statistic(item.getDate());
          stats.put(cal.getTime(), s);
        }
        s.addStat(item.getHour(), item.getReceivedOk(), item.getReceivedFail(), item.getTransmitedOk(), item.getTransmitedFail());
      }

    } catch (Throwable e) {
      logger.error(e,e);
    }
  }

  private static Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      converted = DATE_FORMAT.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  public void setFrom(String from) {
    if (from != null && from.trim().length() > 0)
      filter.setFrom(convertStringToDate(from));
    else {
      long now = System.currentTimeMillis();
      long offset = TimeZone.getDefault().getOffset(now);
      filter.setFrom(new Date(now - offset - 3600000 * 24));
    }
  }

  public String getFrom() {
    return filter.getFrom() == null ? "" : DATE_FORMAT.format(filter.getFrom());
  }

  public void setTill(String till) {
    if (till != null && till.trim().length() > 0)
      filter.setTill(convertStringToDate(till));
    else {
      long now = System.currentTimeMillis();
      long offset = TimeZone.getDefault().getOffset(now);
      filter.setTill(new Date(now - offset));
    }
  }

  public String getTill() {
    return filter.getTill() == null ? "" : DATE_FORMAT.format(filter.getTill());
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public Map getStats() {
    return stats;
  }
}
