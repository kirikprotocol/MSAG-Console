package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.StatQuery;
import ru.novosoft.smsc.infosme.backend.Statistics;
import ru.novosoft.smsc.infosme.backend.TasksStat;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:51:33
 * To change this template use Options | File Templates.
 */

public class TasksStatistics extends InfoSmeBean
{
  public final static String ALL_TASKS_MARKER = "-- ALL TASKS --";
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

    if (getInfoSmeContext().getDataSource() != null)
      stat.setDataSource(getInfoSmeContext().getDataSource());

    stat.setInfoSme(getInfoSmeContext().getInfoSme());

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (getInfoSmeContext().getDataSource() == null)
      warning("DataSource not initialized");

    if (mbQuery != null) {
      try {
        statistics = null;
        if (getInfoSmeContext().getDataSource() != null)
          statistics = stat.getStatistics(query);
        else
          return error("DataSource not initialized");
      } catch (Throwable e) {
        logger.debug("Couldn't get statisctics", e);
        return error("Couldn't get statisctics", e);
      }
      mbQuery = null;
    }

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
    } else {
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
    } else {
      query.setTillDateEnabled(false);
    }
  }

  public String getTillDate()
  {
    return (query.isTillDateEnabled()) ?
            convertDateToString(query.getTillDate()) : "";
  }

  public String getTaskId()
  {
    String id = query.getTaskId();
    return (id == null) ? "" : id;
  }

  public void setTaskId(String taskId)
  {
    query.setTaskId((taskId == null || taskId.length() <= 0 ||
                     taskId.equals(ALL_TASKS_MARKER)) ? null : taskId);
  }

  public String getTaskName(String taskId)
  {
    try {
      return getConfig().getString(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId) + ".name");
    } catch (Throwable e) {
      logger.error("Could not get name for task \"" + taskId + "\"", e);
      error("Could not get name for task \"" + taskId + "\"", e);
      return "";
    }
  }

  public Collection getAllTasks()
  {
    return new SortedList(getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX));
  }
}

