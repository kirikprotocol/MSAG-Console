package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.Date;
import java.util.List;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import ru.novosoft.smsc.infosme.backend.Statistics;
import ru.novosoft.smsc.infosme.backend.StatQuery;
import ru.novosoft.smsc.infosme.backend.TasksStat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:51:33
 * To change this template use Options | File Templates.
 */

public class TasksStatistics extends InfoSmeBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private Statistics statistics = null;
  private StatQuery query = new StatQuery();
  private TasksStat stat = new TasksStat();

  private String mbQuery = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
        return result;

    return RESULT_OK;
  }

  public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    // TODO: init TasksStat with DataSource & InfoSme instances

    if (mbQuery != null) {
      try {
        statistics = null;
        statistics = stat.getStatistics(query);
      } catch (Throwable e) {
        logger.debug("Couldn't get statisctics", e);
        return error("Couldn't get statisctics", e);
      }
      mbQuery = null;
    }

    return RESULT_DONE;
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

  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try
    {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
        converted = formatter.parse(date);
    }
    catch (ParseException e)
    {
        e.printStackTrace();
    }
    return converted;
  }
  private String convertDateToString(Date date) {
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  /* -------------------------- StatQuery delegates -------------------------- */
  public void setFromDate(String fromDate) {
    if (fromDate != null && fromDate.trim().length() >0) {
        query.setFromDate(convertStringToDate(fromDate));
        query.setFromDateEnabled(true);
    } else {
        query.setFromDateEnabled(false);
    }
  }
  public String getFromDate() {
    return (query.isFromDateEnabled()) ?
            convertDateToString(query.getFromDate()) : "";
  }
  public void setTillDate(String tillDate) {
    if (tillDate != null && tillDate.trim().length() >0) {
        query.setTillDate(convertStringToDate(tillDate));
        query.setTillDateEnabled(true);
    } else {
        query.setTillDateEnabled(false);
    }
  }
  public String getTillDate() {
    return (query.isTillDateEnabled()) ?
            convertDateToString(query.getTillDate()) : "";
  }
  public String getTaskId() {
    String id = query.getTaskId();
    return (id == null) ? "":id;
  }
  public void setTaskId(String taskId) {
    query.setTaskId((taskId == null || taskId.length() <= 0) ? taskId:null);
  }
}

