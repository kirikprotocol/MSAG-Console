package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.*;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.Functions;

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
    public final static String ALL_TASKS_MARKER = "-- ALL TASKS --";
    private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

    private Statistics statistics = null;
    private TasksStat  stat  = new TasksStat();
    private StatQuery  query = new StatQuery();

    private String exportFilePath = null;

    private String mbQuery = null;
    private String mbCancel = null;

    private boolean initialized = false;

    protected int init(List errors)
    {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        if (getInfoSmeContext().getDataSource() != null)
            stat.setDataSource(getInfoSmeContext().getDataSource());
        stat.setInfoSme(getInfoSmeContext().getInfoSme());

        if (!initialized) {
            query.setFromDate(Functions.truncateTime(new Date()));
            query.setFromDateEnabled(true);
        }

        return RESULT_OK;
    }

    public int process(HttpServletRequest request)
    {
        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (getInfoSmeContext().getDataSource() == null)
            warning("infosme.error.ds_not_inited");

        if (mbCancel != null) {
            mbCancel = null;
            return RESULT_DONE;
        }
        if (mbQuery != null) {
            try
            {
                statistics = null;
                if (getInfoSmeContext().getDataSource() == null)
                    return error("infosme.error.ds_not_inited");

                statistics = stat.getStatistics(query);
                if (result != RESULT_OK) return result;
            }
            catch (Throwable e) {
                logger.debug("Couldn't get statisctics", e);
                return error("infosme.error.ds_stat_query_failed", e);
            }
            mbQuery = null;
        }
        return RESULT_OK;
    }

    private final static char   COL_SEP       = ',';
    private final static String CAPTION_STR   = "Sibinco InfoSme statistics report";
    private final static String GEN_TIME_STR  = "Generated at:";
    private final static String TASK_STR      = "Task:";
    private final static String FROM_STR      = "From:";
    private final static String TILL_STR      = "Till:";
    private final static String COL_CAP_STR   = "Date/Time,Generated,Delivered,Retried,Failed";

    public int exportStatistics(JspWriter out)
    {
        if (statistics == null) return error("infosme.error.ds_stat_empty");

        try
        {
            String date = null;
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
                String taskName = getTaskName(taskId);
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

        } catch (Exception exc) {
            return error("infosme.error.csv_file", exc);
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
    public String getMbCancel() {
        return mbCancel;
    }
    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }
    public String getExportFilePath() {
        return exportFilePath;
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

    public String getTaskId() {
        String id = query.getTaskId();
        return (id == null) ? "" : id;
    }
    public void setTaskId(String taskId) {
        query.setTaskId((taskId == null || taskId.length() <= 0 ||
                taskId.equals(ALL_TASKS_MARKER)) ? null : taskId);
    }

    public String getTaskName(String taskId)
    {
        try {
            return getConfig().getString(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId) + ".name");
        } catch (Throwable e) {
            logger.error("Could not get name for task \"" + taskId + "\"", e);
            error("infosme.error.task_name_undefined", taskId, e);
            return "";
        }
    }
    public Collection getAllTasks() {
        return new SortedList(getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX));
    }

    public boolean isInitialized() {
        return initialized;
    }
    public void setInitialized(boolean initialized) {
        this.initialized = initialized;
    }
}

