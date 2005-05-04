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

    private static final String DATE_FORMAT = "dd.MM.yyyy";

    private ExportSettings export = new ExportSettings();
    private StatQuery query = new StatQuery();
    private SmsStat stat = null;

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
          if (stat == null) stat = SmsStat.getInstance(appContext);
        } catch (AdminException e) {
          logger.error("Failed to get stat instance", e);
          return error(SMSCErrors.error.smsstat.QueryFailed, e.getMessage());
        }

        if (mbExport != null) {
            try {
                recordsExported = 0;
                if (export == null || export.isEmpty()) recordsExported = stat.exportStatistics(query);
                else recordsExported = stat.exportStatistics(query, export);
            } catch (Exception exc) {
              logger.error("Failed to export stat", exc);
              return error(SMSCErrors.error.smsstat.QueryFailed, exc.getMessage());
            }
        }
        else if (!query.isFromDateEnabled()) {
          query.setFromDate(Functions.truncateTime(new Date()));
          query.setFromDateEnabled(true); query.setTillDateEnabled(false);
        }
        mbExport = null;
        return RESULT_OK;
    }

    public long getRecordsExported() {
        return recordsExported;
    }

    private Date convertStringToDate(String date)
    {
        Date converted = new Date();
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            converted = formatter.parse(date);
        } catch (ParseException e) {
            logger.error("Invaluid date format", e);
        }
        return converted;
    }
    private String convertDateToString(Date date)
    {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      return formatter.format(date);
    }
    public void setDate(String date)
    {
      if (date != null && date.trim().length() > 0) {
        query.setFromDate(convertStringToDate(date));
        query.setFromDateEnabled(true);
      }
      else query.setFromDateEnabled(false);
    }
    public String getDate()
    {
      return (query.isFromDateEnabled()) ?
              convertDateToString(query.getFromDate()) : "";
    }

    public String getSource() {
        return export.getSource();
    }
    public void setSource(String source) {
        export.setSource(source);
    }

    public String getDriver() {
        return export.getDriver();
    }
    public void setDriver(String driver) {
        export.setDriver(driver);
    }

    public String getUser() {
        return export.getUser();
    }
    public void setUser(String user) {
        export.setUser(user);
    }

    public String getPassword() {
        return export.getPassword();
    }
    public void setPassword(String password) {
        export.setPassword(password);
    }

    public String getTablesPrefix() {
        return export.getTablesPrefix();
    }
    public void setTablesPrefix(String tablesPrefix) {
        export.setTablesPrefix(tablesPrefix);
    }
}
