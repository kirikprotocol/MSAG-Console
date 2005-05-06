package ru.novosoft.smsc.jsp.smsstat;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.smsstat.StatQuery;
import ru.novosoft.smsc.admin.smsstat.SmsStat;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Date;
import java.util.Calendar;
import java.util.TimeZone;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 04.05.2005
 * Time: 12:46:19
 * To change this template use File | Settings | File Templates.
 */
public class StatExportBean extends IndexBean
{
    org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(StatExportBean.class);

    private final static String DATE_FORMAT = "dd.MM.yyyy";

    public final static int DEFAULT_DESTINATION = 0;
    public final static int USER_DESTINATION = 1;

    private int destination = DEFAULT_DESTINATION;
    private ExportSettings defaultExport = null;
    private ExportSettings export = new ExportSettings();
    private Date date = Functions.truncateTime(new Date());
    private SmsStat stat = null;

    private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());

    private String mbExport = null;
    private long recordsExported = 0;

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK) return result;
      return RESULT_OK;
    }

    public int process(HttpServletRequest request)
    {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        try {
          if (stat == null) {
              stat = SmsStat.getInstance(appContext);
              defaultExport = stat.getDefaultExportSettings();
          }
        } catch (AdminException e) {
          logger.error("Failed to get stat instance", e);
          return error(SMSCErrors.error.smsstat.QueryFailed, e.getMessage());
        }

        if (mbExport != null)
        {
            recordsExported = 0;
            if (date == null) return error("Please specify date to export");
            try
            {
                StatQuery query = new StatQuery();
                localCaledar.setTime(date);
                localCaledar.set(Calendar.HOUR_OF_DAY, 0); localCaledar.set(Calendar.MINUTE, 0);
                localCaledar.set(Calendar.SECOND, 0); localCaledar.set(Calendar.MILLISECOND, 0);
                Date fromDate = localCaledar.getTime();
                query.setFromDate(fromDate); query.setFromDateEnabled(true);
                localCaledar.add(Calendar.DAY_OF_MONTH, 1);
                Date tillDate = localCaledar.getTime();
                query.setTillDate(tillDate); query.setTillDateEnabled(true);

                if (destination == DEFAULT_DESTINATION || export == null || export.isEmpty())
                     recordsExported = stat.exportStatistics(query);
                else recordsExported = stat.exportStatistics(query, export);

                message("Exported " + recordsExported + " records");
            }
            catch (Exception exc) {
              logger.error("Failed to export stat", exc);
              return error(SMSCErrors.error.smsstat.QueryFailed, exc.getMessage());
            }
        }
        mbExport = null;
        return RESULT_OK;
    }

    public long getRecordsExported() {
        return recordsExported;
    }

    private Date convertStringToDate(String date)
    {
        Date converted = null;
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            converted = formatter.parse(date);
        } catch (ParseException e) {
            logger.error("Invalid date format", e);
        }
        return converted;
    }
    private String convertDateToString(Date date)
    {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      return formatter.format(date);
    }
    public void setDate(String dateStr)
    {
      this.date = (dateStr != null && dateStr.trim().length() > 0) ?
                   convertStringToDate(dateStr) : null;
    }
    public String getDate()
    {
      return ((date != null) ? convertDateToString(date) : "");
    }

    public String getSource() {
        if (destination == DEFAULT_DESTINATION && defaultExport != null && !defaultExport.isEmpty())
            return defaultExport.getSource();
        return (export.getSource() == null) ? "":export.getSource();
    }
    public String getDefaultSource() {
        return (defaultExport == null || defaultExport.getSource() == null) ? "":defaultExport.getSource();
    }
    public void setSource(String source) {
        export.setSource(source);
    }

    public String getDriver() {
        if (destination == DEFAULT_DESTINATION && defaultExport != null && !defaultExport.isEmpty())
            return defaultExport.getDriver();
        return (export.getDriver() == null) ? "":export.getDriver();
    }
    public String getDefaultDriver() {
        return (defaultExport == null || defaultExport.getDriver() == null) ? "":defaultExport.getDriver();
    }
    public void setDriver(String driver) {
        export.setDriver(driver);
    }

    public String getUser() {
        if (destination == DEFAULT_DESTINATION && defaultExport != null && !defaultExport.isEmpty())
            return defaultExport.getUser();
        return (export.getUser() == null) ? "":export.getUser();
    }
    public String getDefaultUser() {
        return (defaultExport == null || defaultExport.getUser() == null) ? "":defaultExport.getUser();
    }
    public void setUser(String user) {
        export.setUser(user);
    }

    public String getPassword() {
        if (destination == DEFAULT_DESTINATION && defaultExport != null && !defaultExport.isEmpty())
            return defaultExport.getPassword();
        return (export.getPassword() == null) ? "":export.getPassword();
    }
    public String getDefaultPassword() {
        return (defaultExport == null || defaultExport.getPassword() == null) ? "":defaultExport.getPassword();
    }
    public void setPassword(String password) {
        export.setPassword(password);
    }

    public String getTablesPrefix() {
        if (destination == DEFAULT_DESTINATION && defaultExport != null && !defaultExport.isEmpty())
            return defaultExport.getTablesPrefix();
        return (export.getTablesPrefix() == null) ? "":export.getTablesPrefix();
    }
    public String getDefaultTablesPrefix() {
        return (defaultExport == null || defaultExport.getTablesPrefix() == null) ? "":defaultExport.getTablesPrefix();
    }
    public void setTablesPrefix(String tablesPrefix) {
        export.setTablesPrefix(tablesPrefix);
    }

    public int getDestination() {
        return destination;
    }
    public void setDestination(int destination) {
        this.destination = destination;
    }

    public String getMbExport() {
        return mbExport;
    }
    public void setMbExport(String mbExport) {
        this.mbExport = mbExport;
    }
}
