package ru.novosoft.smsc.jsp.smsexport;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsexport.SmsExport;
import ru.novosoft.smsc.admin.smsexport.SmsOperativeExport;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.lang.reflect.Method;

public class SmsExportBean extends IndexBean {
    public final static String UNKNOWN_STR = "unknown";
    public final static String ERR_CODES_PREFIX = "smsc.errcode.";
    public final static String ERR_CODE_UNKNOWN = ERR_CODES_PREFIX + UNKNOWN_STR;

    public final static int DEFAULT_DESTINATION = 0;
    public final static int USER_DESTINATION = 1;
    private int destination = DEFAULT_DESTINATION;

    public final static int OPERATIVE_STORAGE = 0;
    public final static int ARCHIVE_STORAGE = 1;
    private int storage = OPERATIVE_STORAGE;

    private Date date = new Date();
    private final static String DATE_FORMAT = "dd.MM.yyyy";
    private SmsExport operative = null;
    private ExportSettings exportSettings = null;
    private ExportSettings defaultExportSettings = null;
    private String source = null;
    private String driver = null;
    private String user = null;
    private String password = null;
    private String tablesPrefix = null;

    private String mbExport = null;


    public class ErrorValue implements Comparable {
        public int errorCode;
        public String errorString;

        public ErrorValue(int errorCode, String errorString) {
            this.errorCode = errorCode;
            this.errorString = errorString;
        }

        public int compareTo(Object o) {
            if (o == null || !(o instanceof ErrorValue)) return -1;
            return (this.errorCode - ((ErrorValue) o).errorCode);
        }
    }

    ;

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK) return result;

        return RESULT_OK;
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_operative"))
            return error(SMSCErrors.error.smsview.AccessDeniedToOperative);

        if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_archive"))
            return error(SMSCErrors.error.smsview.AccessDeniedToArchive);

        Config webConfig = appContext.getConfig();
        Config smscConfig = appContext.getSmsc().getSmscConfig();
        try {
            if ((operative == null) || (storage == OPERATIVE_STORAGE && (!(operative instanceof  SmsOperativeExport))) ||
                    (storage == ARCHIVE_STORAGE && operative instanceof SmsOperativeExport))
                if (storage == OPERATIVE_STORAGE)
                    operative = SmsOperativeExport.getInstance(webConfig, smscConfig);
                else {
                    Class archiveExportClass = Class.forName("ru.novosoft.smsc.admin.smsexport.SmsArchiveExport");
                    Method getArchiveInstance = archiveExportClass.getMethod("getInstance", new Class[] {Config.class, SMSCAppContext.class});
                    operative = (SmsExport) getArchiveInstance.invoke(null, new Object[] {webConfig, appContext});//SmsArchiveExport.getInstance(webConfig, appContext);
                    Method setDate = archiveExportClass.getMethod("setDate", new Class[] {Date.class});
                    setDate.invoke(operative, new Object[] {date});//((SmsArchiveExport) operative).setDate(date);
                }
        } catch (AdminException e) {
            e.printStackTrace();
            return error(SMSCErrors.error.smsexport.ExportFailed, e.getMessage());
        } catch (ClassNotFoundException e) {
            return error(SMSCErrors.error.FeatureIsntInstalled);
        } catch (Exception e) {
            e.printStackTrace();
            return error(SMSCErrors.error.FeatureIsntInstalled);
        }

        defaultExportSettings = operative.getDefaultExportSettings();

        if (source != null && driver != null && user != null && password != null && tablesPrefix != null) {
            exportSettings = new ExportSettings(source, driver, user, password, tablesPrefix);
        }

        if (mbExport != null)
            result = processQuery();

        return result;
    }


    public int processQuery() {
        try {
            operative.export(exportSettings);
            return RESULT_OK;
        } catch (AdminException ex) {
            ex.printStackTrace();
            return error(SMSCErrors.error.smsview.QueryFailed, ex.getMessage());
        }
    }

    public String getMbExport() {
        return mbExport;
    }

    public void setMbExport(String mbExport) {
        this.mbExport = mbExport;
    }

    public String getDriver() {
        return driver;
    }

    public String getPassword() {
        return password;
    }

    public String getSource() {
        return source;
    }

    public String getUser() {
        return user;
    }

    public String getTablesPrefix() {
        return tablesPrefix;
    }

    public ExportSettings getDefaultExportSettings() {
        return defaultExportSettings;
    }

    public int getDestination() {
        return destination;
    }

    public int getStorage() {
        return storage;
    }

    public void setStorage(int storage) {
        this.storage = storage;
    }

    private Date convertStringToDate(String date) {
        Date converted = null;
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            converted = formatter.parse(date);
        } catch (ParseException e) {
            logger.error("Invalid date format", e);
        }
        return converted;
    }

    private String convertDateToString(Date date) {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
        return formatter.format(date);
    }

    public void setDate(String dateStr) {
        this.date = (dateStr != null && dateStr.trim().length() > 0) ?
                convertStringToDate(dateStr) : null;
    }

    public String getDate() {
        return ((date != null) ? convertDateToString(date) : "");
    }

/*  public String getSource() {
      if (destination == DEFAULT_DESTINATION && defaultExportSettings != null && !defaultExportSettings.isEmpty())
          return defaultExportSettings.getSource();
      return (exportSettings.getSource() == null) ? "":exportSettings.getSource();
  }
  public String getDefaultSource() {
      return (defaultExportSettings == null || defaultExportSettings.getSource() == null) ? "":defaultExportSettings.getSource();
  }
  public void setSource(String source) {
      exportSettings.setSource(source);
  }

  public String getDriver() {
      if (destination == DEFAULT_DESTINATION && defaultExportSettings != null && !defaultExportSettings.isEmpty())
          return defaultExportSettings.getDriver();
      return (exportSettings.getDriver() == null) ? "":exportSettings.getDriver();
  }
  public String getDefaultDriver() {
      return (defaultExportSettings == null || defaultExportSettings.getDriver() == null) ? "":defaultExportSettings.getDriver();
  }
  public void setDriver(String driver) {
      exportSettings.setDriver(driver);
  }

  public String getUser() {
      if (destination == DEFAULT_DESTINATION && defaultExportSettings != null && !defaultExportSettings.isEmpty())
          return defaultExportSettings.getUser();
      return (exportSettings.getUser() == null) ? "":exportSettings.getUser();
  }
  public String getDefaultUser() {
      return (defaultExportSettings == null || defaultExportSettings.getUser() == null) ? "":defaultExportSettings.getUser();
  }
  public void setUser(String user) {
      exportSettings.setUser(user);
  }

  public String getPassword() {
      if (destination == DEFAULT_DESTINATION && defaultExportSettings != null && !defaultExportSettings.isEmpty())
          return defaultExportSettings.getPassword();
      return (exportSettings.getPassword() == null) ? "":exportSettings.getPassword();
  }
  public String getDefaultPassword() {
      return (defaultExportSettings == null || defaultExportSettings.getPassword() == null) ? "":defaultExportSettings.getPassword();
  }
  public void setPassword(String password) {
      exportSettings.setPassword(password);
  }

  public String getTablesPrefix() {
      if (destination == DEFAULT_DESTINATION && defaultExportSettings != null && !defaultExportSettings.isEmpty())
          return defaultExportSettings.getTablesPrefix();
      return (exportSettings.getTablesPrefix() == null) ? "":exportSettings.getTablesPrefix();
  }
  public String getDefaultTablesPrefix() {
      return (defaultExportSettings == null || defaultExportSettings.getTablesPrefix() == null) ? "":defaultExportSettings.getTablesPrefix();
  }
  public void setTablesPrefix(String tablesPrefix) {
      exportSettings.setTablesPrefix(tablesPrefix);
  }
*/
}
