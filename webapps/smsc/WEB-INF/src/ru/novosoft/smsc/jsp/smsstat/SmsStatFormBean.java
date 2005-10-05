package ru.novosoft.smsc.jsp.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:38:53 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.smsstat.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspWriter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.io.IOException;

public class SmsStatFormBean extends IndexBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  public static final int RESULT_DLSTAT = SmscBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 11;

  private Statistics statistics = null;
  private StatQuery query = new StatQuery();
  private SmsStat stat = null;

  private boolean includeErrors = false;
  private boolean initialized = false;
  private String mbQuery = null;
  private String mbDetail = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK) return result;

    if (!initialized) {
      query.setFromDate(Functions.truncateTime(new Date()));
      query.setFromDateEnabled(true);
    }

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
     statistics=(Statistics) session.getAttribute("statistics");
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
       // session.removeAttribute("ByRouteId");
        session.setAttribute("ByRouteId", ByRouteId);
       // session.removeAttribute("dateFrom");
        session.setAttribute("statistics", statistics);
        session.setAttribute("dateFrom", dateFrom);
       //  session.removeAttribute("dateTill");
        session.setAttribute("dateTill", dateTill);

      } catch (Exception exc) {
        statistics = null;
        exc.printStackTrace();
        return error(SMSCErrors.error.smsstat.QueryFailed, exc.getMessage());
      }
    } else {
        if (statistics!=null) {this.setFromDate((String) session.getAttribute("dateFrom"));
          this.setTillDate((String) session.getAttribute("dateTill"));
        
        }
    }
    if (mbDetail != null) {
      try {
        mbDetail = null;
        return RESULT_FILTER;
      } catch (Exception exc) {
        return error(exc.getMessage());
      }
    }
    mbQuery = null;
    mbDetail = null;
    return RESULT_OK;
  }

  private final static char   COL_SEP       = ',';
  private final static String CAPTION_STR   = "Sibinco SMSC statistics report,,,,,,,,";
  private final static String GEN_TIME_STR  = "Generated at:,";
  private final static String FROM_STR      = "From:,";
  private final static String TILL_STR      = "Till:,";
  private final static String GEN_STAT_STR  = "General statistics,,,,,,,,";
  private final static String GEN_CAP_STR   = ",Accepted,Rejected,Delivered,Failed,Rescheduled,Temporal,Peak in,Peak out";
  private final static String GEN_TOTAL_STR = "Total SMS processed:,";
  private final static String SME_STAT_STR  = "SME activity,,,,,,,,";
  private final static String RT_STAT_STR   = "Traffic by routes,,,,,,,,";
  private final static String ERR_STAT_STR  = "Error,Count,,,,,,,";
  private final static String ST_STAT_STR   = "SMS delivery state,,,,,,,,";

  private void exportCounters(JspWriter out, CountersSet set) throws IOException
  {
      out.print(set.accepted); out.print(COL_SEP);
      out.print(set.rejected); out.print(COL_SEP);
      out.print(set.delivered); out.print(COL_SEP);
      out.print(set.failed); out.print(COL_SEP);
      out.print(set.rescheduled); out.print(COL_SEP);
      out.print(set.temporal); out.print(COL_SEP);
      out.print(set.peak_i); out.print(COL_SEP);
      out.print(set.peak_o); out.println();
  }
  private void exportErrors(JspWriter out, ExtendedCountersSet set) throws IOException
  {
      if (!includeErrors) return;
      
      out.println(ERR_STAT_STR);
      for (Iterator j = set.getErrors().iterator(); j.hasNext();)
      {
          Object obj_j = j.next();
          if (obj_j == null || !(obj_j instanceof ErrorCounterSet)) continue;
          ErrorCounterSet errStat = (ErrorCounterSet)obj_j;
          String errMessage = getAppContext().getLocaleString("smsc.errcode."+errStat.errcode);
          if (errMessage == null) errMessage = getAppContext().getLocaleString("smsc.errcode.unknown");
          if (errMessage == null) errMessage = "";
          errMessage = ((errMessage == null) ? "":(errMessage+" ")) + ("(" + errStat.errcode + ")");
          out.print("\""+errMessage+"\""); out.print(COL_SEP);
          out.print(errStat.counter); out.print(COL_SEP);
          out.print(COL_SEP); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.print(COL_SEP); out.println(COL_SEP);
      }
  }
  public int exportStatistics(HttpServletRequest request, JspWriter out)
  {
      if (statistics == null) return error("Statistics is empty");

      try
      {   // Header
          String date = null;
          out.println(CAPTION_STR);
          out.print(GEN_TIME_STR); out.print(convertDateToString(new Date()));
          out.print(COL_SEP); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.print(COL_SEP); out.print(COL_SEP);
          out.println(COL_SEP); out.println();
          out.print(FROM_STR);
          date = (query.isFromDateEnabled()) ? convertDateToString(query.getFromDate()):"-";
          out.print(date); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.println(COL_SEP);
          out.print(TILL_STR);
          date = (query.isTillDateEnabled()) ? convertDateToString(query.getTillDate()):"-";
          out.print(date); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.print(COL_SEP); out.print(COL_SEP);
          out.print(COL_SEP); out.println(COL_SEP);
          out.println();

          // General statistics
          out.println(GEN_STAT_STR); out.println(GEN_CAP_STR); out.print(GEN_TOTAL_STR);
          final CountersSet total = statistics.getTotal();
          exportCounters(out, total);
          SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy");
          for (Iterator i = statistics.getDateStat().iterator(); i.hasNext();)
          {
              Object obj_i = i.next();
              if (obj_i == null || !(obj_i instanceof DateCountersSet)) continue;
              DateCountersSet dateStat = (DateCountersSet)obj_i;
              date = formatter.format(dateStat.getDate());
              out.print(date); out.print(COL_SEP);
              exportCounters(out, dateStat);

              for (Iterator j = dateStat.getHourStat().iterator(); j.hasNext();)
              {
                  Object obj_j = j.next();
                  if (obj_j == null || !(obj_j instanceof HourCountersSet)) continue;
                  HourCountersSet hourStat = (HourCountersSet)obj_j;
                  out.print(hourStat.getHour()); out.print(COL_SEP);
                  exportCounters(out, hourStat);
              }
          }
          //final Locale locale = request.getLocale();

          // SME activity
          out.println();
          out.println(SME_STAT_STR);
          out.println("SME Id"+GEN_CAP_STR);
          for (Iterator i = statistics.getSmeIdStat().iterator(); i.hasNext();)
          {
              Object obj_i = i.next();
              if (obj_i == null || !(obj_i instanceof SmeIdCountersSet)) continue;
              SmeIdCountersSet smeStat = (SmeIdCountersSet)obj_i;
              out.print("\""+smeStat.smeid+"\""); out.print(COL_SEP);
              exportCounters(out, smeStat);
              exportErrors(out, smeStat);
          }

          // Traffic by routes
          out.println();
          out.println(RT_STAT_STR);
          out.println("Route Id"+GEN_CAP_STR);
          for (Iterator i = statistics.getRouteIdStat().iterator(); i.hasNext();)
          {
              Object obj_i = i.next();
              if (obj_i == null || !(obj_i instanceof RouteIdCountersSet)) continue;
              RouteIdCountersSet routeStat = (RouteIdCountersSet)obj_i;
              out.print("\""+routeStat.routeid+"\""); out.print(COL_SEP);
              exportCounters(out, routeStat);
              exportErrors(out, routeStat);
          }

          if (includeErrors) {
              // SMS delivery state
              out.println();
              out.println(ST_STAT_STR);
              exportErrors(out, statistics);
          }

      } catch (Exception exc) {
        return error("CSV File format error", exc);
      }
      return RESULT_OK;

  }

  public Statistics getStatistics() {
    return statistics;
  }

  public String getMbQuery() {
    return mbQuery;
  }
  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public String getMbDetail() {
    return mbDetail;
  }
  public void setMbDetail(String mbDetail) {
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

  public String getFromDate() {
    return (query.isFromDateEnabled()) ? convertDateToString(query.getFromDate()) : "";
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

  public String getTillDate() {
    return (query.isTillDateEnabled()) ?  convertDateToString(query.getTillDate()) : "";
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public boolean isIncludeErrors() {
    return includeErrors;
  }
  public void setIncludeErrors(boolean includeErrors) {
    this.includeErrors = includeErrors;
  }

}

