package ru.novosoft.smsc.jsp.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:38:53 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.smsstat.StatQuery;
import ru.novosoft.smsc.admin.smsstat.Statistics;
import ru.novosoft.smsc.admin.smsstat.SmsStat;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collection;
import java.util.Date;
import java.util.List;

public class SmsStatFormBean extends IndexBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private Statistics statistics = null;
  private StatQuery query = new StatQuery();
  private SmsStat stat = null;

  private String mbQuery = null;
  private String mbDetail = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;
    try {
      if (stat == null)
          stat = SmsStat.getInstance(appContext.getSmsc().getSmscConfig(), appContext.getConfig());
    } catch (AdminException e) {
      return error(SMSCErrors.error.smsstat.QueryFailed, e.getMessage());
    }

    if (mbQuery != null) {
      try {
        statistics = stat.getStatistics(query);
        String dateFrom = this.getFromDate();
        String dateTill = this.getTillDate();

        Collection ByRouteId = statistics.getRouteIdStat();
        session.removeAttribute("ByRouteId");
        session.setAttribute("ByRouteId", ByRouteId);
        session.removeAttribute("dateFrom");
        session.setAttribute("dateFrom", dateFrom);
        session.removeAttribute("dateTill");
        session.setAttribute("dateTill", dateTill);

      } catch (Exception exc) {
        statistics = null;
        exc.printStackTrace();
        return error(SMSCErrors.error.smsstat.QueryFailed, exc.getMessage());
      }
    }
    else if (!query.isFromDateEnabled()) {
      query.setFromDate(Functions.truncateTime(new Date()));
      query.setFromDateEnabled(true);
    }
    if (mbDetail != null) {
      try {
        mbDetail = null;
        return RESULT_FILTER;
      } catch (Exception exc) {
        return error(exc.getMessage());
      }
    }
    else if (!query.isFromDateEnabled()) {
      //  query.setFromDate(Functions.truncateTime(new Date()));
      //  query.setFilterFromDate(true);
    }
    mbQuery = null;
    mbDetail = null;
    return RESULT_OK;
  }

  public Statistics getStatistics()
  {
    return statistics;
  }

  public String getMbQuery()
  {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery)
  {
    this.mbQuery = mbQuery;
  }

  public String getMbDetail()
  {
    return mbDetail;
  }

  public void setMbDetail(String mbDetail)
  {
    this.mbDetail = mbDetail;
  }

  private Date convertStringToDate(String date)
  {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  private String convertDateToString(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  /* -------------------------- StatQuery delegates -------------------------- */
  public void setFromDate(String fromDate)
  {
    if (fromDate != null && fromDate.trim().length() > 0) {
      query.setFromDate(convertStringToDate(fromDate));
      query.setFromDateEnabled(true);
    }
    else {
      query.setFromDateEnabled(false);
    }
  }

  public String getFromDate()
  {
    return (query.isFromDateEnabled()) ?
            convertDateToString(query.getFromDate()) : "";
  }

  public void setTillDate(String tillDate)
  {
    if (tillDate != null && tillDate.trim().length() > 0) {
      query.setTillDate(convertStringToDate(tillDate));
      query.setTillDateEnabled(true);
    }
    else {
      query.setTillDateEnabled(false);
    }
  }

  public String getTillDate()
  {
    return (query.isTillDateEnabled()) ?
            convertDateToString(query.getTillDate()) : "";
  }

}
