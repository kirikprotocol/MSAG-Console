package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.io.*;
import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:42:34 PM
 */
public class DbsmeBean extends IndexBean
{

  public static final int RESULT_OVERVIEW = IndexBean.PRIVATE_RESULT;
  public static final int RESULT_PARAMS = IndexBean.PRIVATE_RESULT + 1;
  public static final int RESULT_DRIVERS = IndexBean.PRIVATE_RESULT + 2;
  public static final int RESULT_PROVIDERS = IndexBean.PRIVATE_RESULT + 3;
  public static final int RESULT_PROVIDER = IndexBean.PRIVATE_RESULT + 4;
  protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 5;

  private String menuSelection = null;
  protected Config config = null;
  private DbSmeContext context = null;
  private SmeTransport smeTransport = null;
  private File originalConfigFile = null;
  private File tempConfigFile = new File(WebAppFolders.getWorkFolder(), Constants.DBSME_SME_ID + ".config.xml.tmp");

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      context = DbSmeContext.getInstance(getAppContext());
    } catch (AdminException e) {
      logger.error("Could not instantiate DbSme context", e);
      return error("Could not instantiate DbSme context", e);
    }
    smeTransport = context.getSmeTransport();

    try {
      originalConfigFile = new File(appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getServiceFolder(), "conf/config.xml");
      if (!tempConfigFile.exists()) {
        if (!originalConfigFile.exists()) {
          logger.error("Couldn't find DBSME config file (" + originalConfigFile.getAbsolutePath() + ")");
          return error(DBSmeErrors.error.couldntFindConfig, originalConfigFile.getAbsolutePath());
        }
        InputStream in = new BufferedInputStream(new FileInputStream(originalConfigFile));
        OutputStream out = new BufferedOutputStream(new FileOutputStream(tempConfigFile));
        for (int readedByte = in.read(); readedByte >= 0; readedByte = in.read())
          out.write(readedByte);
        in.close();
        out.close();
      }

      config = new Config(tempConfigFile);
    } catch (Throwable e) {
      logger.error("Couldn't get DBSME config", e);
      return error(DBSmeErrors.error.couldntGetConfig, e);
    }

    return RESULT_OK;
  }

  protected File getOriginalConfigFile()
  {
    return originalConfigFile;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if ("overview".equalsIgnoreCase(menuSelection))
      return RESULT_OVERVIEW;
    if ("params".equalsIgnoreCase(menuSelection))
      return RESULT_PARAMS;
    if ("drivers".equalsIgnoreCase(menuSelection))
      return RESULT_DRIVERS;
    if ("providers".equalsIgnoreCase(menuSelection))
      return RESULT_PROVIDERS;

    return RESULT_OK;
  }

  public byte getServiceStatus()
  {
    try {
      return appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getStatus();
    } catch (AdminException e) {
      logger.error("Couldn't get DBSME status, nested: " + e.getMessage(), e);
      return ServiceInfo.STATUS_UNKNOWN;
    }
  }

  public Set getParameterNames()
  {
    return config.getParameterNames();
  }

  public int getInt(String paramName)
  {
    try {
      return config.getInt(paramName);
    } catch (Exception e) {
      logger.debug("Couldn't get int parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
      return 0;
    }
  }

  public String getString(String paramName)
  {
    try {
      return config.getString(paramName);
    } catch (Exception e) {
      logger.debug("Couldn't get string parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
      return "";
    }
  }

  public String getOptionalString(String paramName)
  {
    try {
      return config.getString(paramName);
    } catch (Exception e) {
      return "";
    }
  }

  public boolean getBool(String paramName)
  {
    try {
      return config.getBool(paramName);
    } catch (Exception e) {
      logger.debug("Couldn't get boolean parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
      return false;
    }
  }

  public boolean getOptionalBool(String paramName)
  {
    try {
      return config.getBool(paramName);
    } catch (Exception e) {
      return false;
    }
  }

  public Object getParameter(String paramName)
  {
    return config.getParameter(paramName);
  }

  public String getMenuSelection()
  {
    return menuSelection;
  }

  public void setMenuSelection(String menuSelection)
  {
    this.menuSelection = menuSelection;
  }

  public Set getSectionChildSectionNames(String sectionName)
  {
    return config.getSectionChildSectionNames(sectionName);
  }

  protected DbSmeContext getContext()
  {
    return context;
  }

  protected File getTempConfigFile()
  {
    return tempConfigFile;
  }

  protected SmeTransport getSmeTransport()
  {
    return smeTransport;
  }
}
