package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.DateCountersSet;
import ru.novosoft.smsc.infosme.backend.HourCountersSet;
import ru.novosoft.smsc.infosme.backend.Statistics;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.tables.stat.*;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskArchiveDataSource;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.jsp.JspWriter;
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
    public static final int RESULT_EXPORT = PRIVATE_RESULT + 1;

    public final static String ALL_TASKS_MARKER = "-- ALL TASKS --";
    public static final int VIEW_DATES = 0;
    public static final int VIEW_TASKS = 1;

    private static final SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    private Statistics statistics = null;
    private StatQuery  query = new StatQuery();

    private String mbQuery = null;
    private String mbExport = null;
    private String mbCancel = null;
    private int view = VIEW_DATES;

    private StatisticsDataSource ds;
    private TaskStatTableHelper tasksStatsTable;

    private boolean initialized = false;

    private String archiveDate;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

      try {
        String serviceFolder = appContext.getHostsManager().getServiceInfo("InfoSme").getServiceFolder().getAbsolutePath();
        String statsStoreDir = getInfoSmeConfig().getStatStoreLocation();
	if( statsStoreDir.length() > 0 && statsStoreDir.charAt(0) != '/' )
	  statsStoreDir = serviceFolder + '/' + statsStoreDir;
        ds = new StatisticsDataSource(statsStoreDir);
      } catch (Exception e) {
        return error("Can't init dataa source", e);
      }
      if(query.getTaskId() != null && archiveDate != null && archiveDate.length() != 0) {
        query.setActive(false);
      }

        return RESULT_OK;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

      query.setConfig(getConfig());
      if (!isUserAdmin(request))
        query.setOwner(request.getRemoteUser());

      try {
        if (mbCancel != null) {
          mbCancel = null;
          return RESULT_DONE;
        }

        if (mbQuery != null) {
          initialized = true;
          mbQuery = null;
        } else if (mbExport != null) {
          mbExport = null;
//          processDatesQuery(request);
          return RESULT_EXPORT;
        }

        if (initialized) {
          if (view == VIEW_DATES)
            processDatesQuery(request);
          else
            processTasksQuery(request);
        }
      } catch (Throwable e) {
        logger.debug("Couldn't get statisctics", e);
        return error("infosme.error.ds_stat_query_failed", e);
      }
      return RESULT_OK;
    }

    private void processTasksQuery(HttpServletRequest request) throws AdminException {
      tasksStatsTable = new TaskStatTableHelper("tasksStatistics", ds, query);
      try {
        tasksStatsTable.setPageSize(20);
        tasksStatsTable.processRequest(request);
        tasksStatsTable.fillTable();
      } catch (TableHelperException e) {
        throw new AdminException(e.getMessage(), e);
      }
    }

    private void processDatesQuery(HttpServletRequest request) throws AdminException {
     flushStatistics(query);

      statistics = null;
      statistics = new Statistics();

//      query.setConfig(getConfig());
//      if (!isUserAdmin(request))
//        query.setOwner(request.getRemoteUser());

      // Convert time to local
      final User user = getUser(request);
      if (user != null) {
        if (query.getFromDate() != null)
          query.setFromDate(user.getLocalTime(query.getFromDate()));
        if (query.getTillDate() != null)
          query.setTillDate(user.getLocalTime(query.getTillDate()));
      }

      StatisticsVisitor v = new StatisticsVisitor(user);
      ds.visit(v, query);

      // Convert time back to user time
      if (user != null) {
        if (query.getFromDate() != null)
          query.setFromDate(user.getUserTime(query.getFromDate()));
        if (query.getTillDate() != null)
          query.setTillDate(user.getUserTime(query.getTillDate()));
      }

      if (v.getDateCounters() != null)
        statistics.addDateStat(v.getDateCounters());
    }

    private void flushStatistics(StatQuery query) throws AdminException {
      boolean needFlush = true;
      if (query.getTillDate() != null) {
        long till = query.getTillDate().getTime();
        long curr = (new Date()).getTime();
        needFlush = (till >= curr - 3600);
      }

      if (needFlush)
          getInfoSme().flushStatistics();
    }

    private static boolean needChangeDate(Calendar oldPeriod, Calendar newPeriod) {
      return oldPeriod.get(Calendar.YEAR) != newPeriod.get(Calendar.YEAR) ||
             oldPeriod.get(Calendar.MONTH) != newPeriod.get(Calendar.MONTH) ||
             oldPeriod.get(Calendar.DAY_OF_MONTH) != newPeriod.get(Calendar.DAY_OF_MONTH);
    }



    private final static char   COL_SEP       = ';';
    private final static String CAPTION_STR   = "Sibinco InfoSme statistics report";
    private final static String GEN_TIME_STR  = "Generated at:";
    private final static String TASK_STR      = "Task:";
    private final static String FROM_STR      = "From:";
    private final static String TILL_STR      = "Till:";
    private final static String COL_CAP_STR   = "Date/Time;Generated;Delivered;Retried;Failed";

    public void exportStatistics(HttpServletRequest request, JspWriter out)
    {
        try
        {
            if (view == VIEW_DATES) {
            processDatesQuery(request);
            if (statistics == null) {
              logger.warn("Statistics is empty.");
              return;
            }
            String date;
            out.print(CAPTION_STR);
            out.print(COL_SEP); out.print(COL_SEP);
            out.print(COL_SEP); out.println(COL_SEP);
            out.print(GEN_TIME_STR); out.print(COL_SEP);
            out.print(convertDateToString(new Date())); out.print(COL_SEP);
            out.print(COL_SEP); out.println(COL_SEP);
            out.println();
            out.print(TASK_STR); out.print(COL_SEP);
            String taskId = query.getTaskId();
            if ((taskId == null || taskId.length() <= 0)) out.print("All");
            else {
              Task t;
              if(archiveDate != null && archiveDate.length() !=0) {
                try{
                  TaskArchiveDataSource tds = new TaskArchiveDataSource(getInfoSme(), getInfoSmeConfig().getArchiveDir());
                  t = tds.get(new SimpleDateFormat("ddMMyyyy").parse(archiveDate), getTaskId());
                }catch (Exception e){
                  logger.error(e,e);
                  e.printStackTrace();
                  return;
                }
              }else {
                t = getInfoSmeConfig().getTask(taskId);
              }
              String taskName = t.getName();
              out.print((taskName == null || taskName.length() <= 0) ? "???":taskName);
            }

            out.print(COL_SEP); out.print(COL_SEP); out.println(COL_SEP);
            out.print(FROM_STR); out.print(COL_SEP);
            date = (query.isFromDateEnabled()) ? convertDateToString(query.getFromDate()):"-";
            out.print(date); out.print(COL_SEP);
            out.print(COL_SEP); out.println(COL_SEP);
            out.print(TILL_STR); out.print(COL_SEP);
            date = (query.isTillDateEnabled()) ? convertDateToString(query.getTillDate()):"-";
            out.print(date); out.print(COL_SEP);
            out.print(COL_SEP); out.println(COL_SEP);
            out.println();
            out.print(COL_CAP_STR); out.println();

            SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy");
            for (Iterator i = statistics.getCountersByDates().iterator(); i.hasNext();)
            {
              DateCountersSet dateCounters =    (DateCountersSet) i.next();
              date = formatter.format(dateCounters.getDate());
              out.print(date); out.print(COL_SEP);
              out.print(dateCounters.generated); out.print(COL_SEP);
              out.print(dateCounters.delivered); out.print(COL_SEP);
              out.print(dateCounters.retried); out.print(COL_SEP);
              out.print(dateCounters.failed); out.println();

              for (Iterator j = dateCounters.getHourStat().iterator(); j.hasNext();)
              {
                HourCountersSet hourCounters = (HourCountersSet) j.next();
                out.print(hourCounters.getHour()); out.print(COL_SEP);
                out.print(hourCounters.generated); out.print(COL_SEP);
                out.print(hourCounters.delivered); out.print(COL_SEP);
                out.print(hourCounters.retried); out.print(COL_SEP);
                out.print(hourCounters.failed); out.println();
              }
            }
            } else {
              processTasksQuery(request);
              tasksStatsTable.exportCsv(out);
            }

          System.out.println("Export finished");

        } catch (Exception exc) {
          logger.error("Unable to export statistics", exc);
        }
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
    public String getMbCancel() {
        return mbCancel;
    }
    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

  public String getMbExport() {
    return mbExport;
  }

  public void setMbExport(String mbExport) {
    this.mbExport = mbExport;
  }

  private static Date convertStringToDate(String date)
    {
        Date converted;
        try {
            synchronized(formatter) {
              converted = formatter.parse(date);
            }
        } catch (ParseException e) {
            e.printStackTrace();
            converted = new Date();
        }
        return converted;
    }
    private static String convertDateToString(Date date)
    {
        synchronized(formatter) {
          return formatter.format(date);
        }
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

    public String getTaskId() {
        String id = query.getTaskId();
        return (id == null) ? "" : id;
    }

    public void setTaskId(String taskId) {
        query.setTaskId((taskId == null || taskId.length() <= 0 || taskId.equals(ALL_TASKS_MARKER)) ? null : taskId);
    }

    public String getTaskName() {
      Task t;
      if(archiveDate != null && archiveDate.length() != 0) {
        try{
          TaskArchiveDataSource tds = new TaskArchiveDataSource(getInfoSme(), getInfoSmeConfig().getArchiveDir());
          t = tds.get(new SimpleDateFormat("ddMMyyyy").parse(archiveDate), getTaskId());
          return t.getName();
        }catch (Exception e){
          logger.error(e,e);
          return null;
        }
      }
      return query.getTaskId() == null || query.getTaskId().length() <= 0 || query.getTaskId().equals(ALL_TASKS_MARKER)
          ? null :
          (t = getInfoSmeConfig().getTask(query.getTaskId())) == null ? null : t.getName();
    }

    public int getView() {
      return view;
    }

    public void setView(int view) {
      this.view = view;
    }

  public boolean isInitialized() {
        return initialized;
    }
    public void setInitialized(boolean initialized) {
        this.initialized = initialized;
    }

  public TaskStatTableHelper getTasksStatsTable() {
    return tasksStatsTable;
  }

  public String getArchiveDate() {
    return archiveDate != null ? archiveDate : "";
  }

  public void setArchiveDate(String archiveDate) {
    this.archiveDate = archiveDate;
  }

  private class StatisticsVisitor implements StatVisitor {

    final HashMap hourCounterSets = new HashMap(100);
    private final User user;

    Calendar oldPeriod = null;
    DateCountersSet dateCounters = null;

    private StatisticsVisitor(User user) {
      this.user = user;
    }

    public DateCountersSet getDateCounters() {
      return dateCounters;
    }

    public boolean visit(StatisticDataItem st) {

      final Calendar newPeriod = Calendar.getInstance();

      newPeriod.setTime(user.getUserTime((Date)st.getValue("period")));

      int hour = newPeriod.get(Calendar.HOUR_OF_DAY);

      HourCountersSet s = new HourCountersSet(((Integer)st.getValue("generated")).intValue(),
          ((Integer)st.getValue("delivered")).intValue(),
          ((Integer)st.getValue("retried")).intValue(),
          ((Integer)st.getValue("failed")).intValue(),hour);

      HourCountersSet hourCounters = (HourCountersSet)hourCounterSets.get(newPeriod.getTime());
      if (hourCounters == null) {
        hourCounterSets.put(newPeriod.getTime(), s);

        if (dateCounters == null) { // on first iteration
          dateCounters = new DateCountersSet(newPeriod.getTime());
        } else if (needChangeDate(oldPeriod, newPeriod)) { // on date changed
          statistics.addDateStat(dateCounters);
          dateCounters = new DateCountersSet(newPeriod.getTime());
        }
        dateCounters.addHourStat(s);
      } else {
        if (dateCounters != null)
          dateCounters.increment(s);
        hourCounters.increment(s);
      }

      oldPeriod = newPeriod;
      return true;
    }
  }
}

