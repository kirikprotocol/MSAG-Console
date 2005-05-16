package ru.novosoft.smsc.jsp.smsexport;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsexport.SmsExport;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsView;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

public class SmsExportBean extends IndexBean
{
  public final static String UNKNOWN_STR = "unknown";
  public final static String ERR_CODES_PREFIX = "smsc.errcode.";
  public final static String ERR_CODE_UNKNOWN = ERR_CODES_PREFIX + UNKNOWN_STR;

  public final static int DEFAULT_DESTINATION = 0;
  public final static int USER_DESTINATION = 1;
  private int destination = DEFAULT_DESTINATION;


  private SmsExport operative = null;
  private ExportSettings exportSettings = null;
  private ExportSettings defaultExportSettings = null;
  private String source = null;
  private String driver = null;
  private String user = null;
  private String password = null;
  private String tablesPrefix = null;

  private String mbExport = null;


  public class ErrorValue implements Comparable
  {
    public int errorCode;
    public String errorString;

    public ErrorValue(int errorCode, String errorString)
    {
      this.errorCode = errorCode;
      this.errorString = errorString;
    }

    public int compareTo(Object o)
    {
      if (o == null || !(o instanceof ErrorValue)) return -1;
      return (this.errorCode - ((ErrorValue) o).errorCode);
    }
  };

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

    if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_operative"))
      return error(SMSCErrors.error.smsview.AccessDeniedToOperative);

      Config webConfig = appContext.getConfig(); // webappConfig
      Config smscConfig=appContext.getSmsc().getSmscConfig();
    try {
      if (operative == null) operative = SmsExport.getInstance(webConfig,smscConfig);
    } catch (AdminException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      return error(SMSCErrors.error.smsexport.ExportFailed, e.getMessage());
    }
    defaultExportSettings=operative.getDefaultExportSettings();

    if (source!=null &&  driver!=null && user!=null && password!=null && tablesPrefix!=null)
    {
     exportSettings=new ExportSettings(source,driver,user,password,tablesPrefix);
    }
    if (mbExport != null)
      result = processQuery();

    return result;
  }


  public int processQuery()
  {
    try {
      operative.export(exportSettings);
      return RESULT_OK;
    } catch (AdminException ex) {
      ex.printStackTrace();
      return error(SMSCErrors.error.smsview.QueryFailed, ex.getMessage());
    }
  }

  public String getMbExport()
  {
    return mbExport;
  }

  public void setMbExport(String mbExport)
  {
    this.mbExport = mbExport;
  }

  public String getDriver()
  {
    return driver;
  }

  public String getPassword()
  {
    return password;
  }

  public String getSource()
  {
    return source;
  }

  public String getUser()
  {
    return user;
  }

  public String getTablesPrefix()
  {
    return tablesPrefix;
  }

  public ExportSettings getDefaultExportSettings()
  {
    return defaultExportSettings;
  }

  public int getDestination()
  {
    return destination;
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
