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

  private SmsView view = new SmsView();
  private SmsExport operative = null;
  private ExportSettings exportSettings = null;
  private ExportSettings defaultExportSettings = null;
  private String source = null;
  private String driver = null;
  private String user = null;
  private String password = null;
  private String tablesPrefix = null;

  //private String mbClear = null;
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
     Config webConfig = appContext.getConfig(); // webappConfig
     try {
         final String section = "opersave_datasource";
         final String source = webConfig.getString(section + ".source");
         final String driver = webConfig.getString(section + ".driver");
         final String user   = webConfig.getString(section + ".user");
         final String pass   = webConfig.getString(section + ".pass");
         final String prefix = webConfig.getString(section + ".tables_prefix");
         defaultExportSettings = new ExportSettings(source, driver, user, pass, prefix);
     } catch (Exception e) {
         return error("Failed to configure default export settings. Details: " + e.getMessage());
     }
    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    try {
      if (operative == null) operative = SmsExport.getInstance(appContext);
    } catch (AdminException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      return error(SMSCErrors.error.smsexport.ExportFailed, e.getMessage());
    }

    if (!request.isUserInRole("smsView") && !request.isUserInRole("smsView_operative"))
      return error(SMSCErrors.error.smsview.AccessDeniedToOperative);

    if (source!=null &&  driver!=null && user!=null && password!=null && tablesPrefix!=null)
    {
     exportSettings=new ExportSettings(source,driver,user,password,tablesPrefix);
    }
    if (mbExport != null)
      result = processQuery();
    // else if (mbClear != null) result = clearQuery();
    // else result = processResortAndNavigate(false);

    // mbClear = null;

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
}
