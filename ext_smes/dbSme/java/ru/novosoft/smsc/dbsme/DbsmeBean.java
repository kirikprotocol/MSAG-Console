package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.*;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.List;
import java.util.Set;

import org.xml.sax.SAXException;

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

  private String smeId = "dbSme";
  private String menuSelection = null;
  protected Config config = null;
  private DbSmeContext context = null;
  private SmeTransport smeTransport = null;
  private File originalConfigFile = null;
  private File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");

  protected synchronized void restoreFromOriginalConfig()
      throws IOException, ParserConfigurationException, SAXException
  {
      if (tempConfigFile.exists()) tempConfigFile.delete();

      InputStream in = new BufferedInputStream(new FileInputStream(originalConfigFile));
      OutputStream out = new BufferedOutputStream(new FileOutputStream(tempConfigFile));
      for (int readedByte = in.read(); readedByte >= 0; readedByte = in.read())
        out.write(readedByte);
      in.close(); out.close();

      config = new Config(tempConfigFile);
  }
  protected int init(List errors)
  {
    tempConfigFile = new File(WebAppFolders.getWorkFolder(), smeId + ".config.xml.tmp");

    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      originalConfigFile = new File(appContext.getHostsManager().getServiceInfo(smeId).getServiceFolder(), "conf/config.xml");
      if (!tempConfigFile.exists()) {
        if (!originalConfigFile.exists()) {
            logger.error("Couldn't find DBSme config file (" + originalConfigFile.getAbsolutePath() + ")");
            return error("dbsme.error.no_config", originalConfigFile.getAbsolutePath());
        }
        restoreFromOriginalConfig();
      }
      else {
        config = new Config(tempConfigFile);
      }

      try {
        context = DbSmeContext.getInstance(getAppContext(), smeId,
                                           config.getString("DBSme.Admin.host"),
                                           config.getInt("DBSme.Admin.port"));
      } catch (AdminException e) {
        logger.error("Could not instantiate DBSme context", e);
        return error("dbsme.error.ctx_inst", e);
      }
      smeTransport = context.getSmeTransport();

    } catch (Throwable e) {
      logger.error("Couldn't get DBSme config", e);
      return error("dbsme.error.no_config", e);
    }

    return RESULT_OK;
  }

  protected File getOriginalConfigFile()
  {
    return originalConfigFile;
  }

  public int process(HttpServletRequest request)
  {
    try {
      smeId = Functions.getServiceId(request.getServletPath());
    } catch (AdminException e) {
      logger.error("Could not discover sme id", e);
      error("dbsme.error.no_smeid", e);
    }

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

  public boolean isOnline() {
      try {
        return appContext.getHostsManager().getServiceInfo(smeId).isOnline();
      } catch (AdminException e) {
        logger.error("Couldn't get DBSME status, nested: " + e.getMessage(), e);
        return false;
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

  protected int setProviderEnabled(String providerName, boolean enable)
  {
    if (isOnline()) {
      try {
        getSmeTransport().setProviderEnabled(providerName, enable);
      } catch (AdminException e) {
        logger.error("Could not " + (enable ? "enable" : "disable") + " provider \"" + providerName + '"', e);
        return error(enable ? "dbsme.error.provider_enable":
                              "dbsme.error.provider_disable", " provider \"" + providerName + '"', e);
      }
    }
    config.setBool(Provider.createProviderPrefix(providerName) + ".enabled", enable);
    return RESULT_DONE;
  }

  protected boolean isProviderEquals(String providerName, String oldProviderName, String address, int connections,
                                     String dbInstance, String dbUserName, String dbUserPassword, String type, boolean watchdog,
                                     String service_not_available, String job_not_found, String ds_failure, String ds_connection_lost, String ds_statement_fail,
                                     String query_null, String input_parse, String output_format, String invalid_config)
  {
    String prefix = createProviderPrefix(providerName);
    if (logger.isDebugEnabled()) {
      logger.debug("address (\"" + address + "\"):" + config.isStringParamEquals(prefix + ".address", address));
      logger.debug("connections (\"" + connections + "\"):" + config.isIntParamEquals(prefix + ".DataSource.connections", connections));
      logger.debug("dbInstance (\"" + dbInstance + "\"):" + config.isStringParamEquals(prefix + ".DataSource.dbInstance", dbInstance));
      logger.debug("dbUserName (\"" + dbUserName + "\"):" + config.isStringParamEquals(prefix + ".DataSource.dbUserName", dbUserName));
      logger.debug("dbUserPassword (\"" + dbUserPassword + "\"):" + config.isStringParamEquals(prefix + ".DataSource.dbUserPassword", dbUserPassword));
      logger.debug("type (\"" + type + "\"):" + config.isStringParamEquals(prefix + ".DataSource.type", type));
      logger.debug("watchdog (\"" + watchdog + "\"):" + config.isBooleanParamEquals(prefix + ".DataSource.watchdog", watchdog));
      logger.debug("service_not_available (\"" + service_not_available + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.SERVICE_NOT_AVAIL", service_not_available));
      logger.debug("job_not_found (\"" + job_not_found + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.JOB_NOT_FOUND", job_not_found));
      logger.debug("ds_failure (\"" + ds_failure + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.DS_FAILURE", ds_failure));
      logger.debug("ds_connection_lost (\"" + ds_connection_lost + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost));
      logger.debug("ds_statement_fail (\"" + ds_statement_fail + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail));
      logger.debug("query_null (\"" + query_null + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.QUERY_NULL", query_null));
      logger.debug("input_parse (\"" + input_parse + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.INPUT_PARSE", input_parse));
      logger.debug("output_format (\"" + output_format + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.OUTPUT_FORMAT", output_format));
      logger.debug("invalid_config (\"" + invalid_config + "\"):" + config.isStringParamEquals(prefix + ".MessageSet.INVALID_CONFIG", invalid_config));
    }
    return providerName.equals(oldProviderName)
           && config.isStringParamEquals(prefix + ".address", address)
           && config.isIntParamEquals(prefix + ".DataSource.connections", connections)
           && config.isStringParamEquals(prefix + ".DataSource.dbInstance", dbInstance)
           && config.isStringParamEquals(prefix + ".DataSource.dbUserName", dbUserName)
           && config.isStringParamEquals(prefix + ".DataSource.dbUserPassword", dbUserPassword)
           && config.isStringParamEquals(prefix + ".DataSource.type", type)
           && config.isBooleanParamEquals(prefix + ".DataSource.watchdog", watchdog)

           && config.isStringParamEquals(prefix + ".MessageSet.SERVICE_NOT_AVAIL", service_not_available)
           && config.isStringParamEquals(prefix + ".MessageSet.JOB_NOT_FOUND", job_not_found)
           && config.isStringParamEquals(prefix + ".MessageSet.DS_FAILURE", ds_failure)
           && config.isStringParamEquals(prefix + ".MessageSet.DS_CONNECTION_LOST", ds_connection_lost)
           && config.isStringParamEquals(prefix + ".MessageSet.DS_STATEMENT_FAIL", ds_statement_fail)
           && config.isStringParamEquals(prefix + ".MessageSet.QUERY_NULL", query_null)
           && config.isStringParamEquals(prefix + ".MessageSet.INPUT_PARSE", input_parse)
           && config.isStringParamEquals(prefix + ".MessageSet.OUTPUT_FORMAT", output_format)
           && config.isStringParamEquals(prefix + ".MessageSet.INVALID_CONFIG", invalid_config);
  }

  protected boolean isProviderEquals(Config originalConfig, String providerName)
  {
    String prefix = createProviderPrefix(providerName);
    return config.isParamEquals(originalConfig, prefix + ".address")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.connections")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.dbInstance")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.dbUserName")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.dbUserPassword")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.type")
           && config.isParamEquals(originalConfig, prefix + ".DataSource.watchdog")

           && config.isParamEquals(originalConfig, prefix + ".MessageSet.JOB_NOT_FOUND")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.DS_FAILURE")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.DS_CONNECTION_LOST")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.DS_STATEMENT_FAIL")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.QUERY_NULL")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.INPUT_PARSE")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.OUTPUT_FORMAT")
           && config.isParamEquals(originalConfig, prefix + ".MessageSet.INVALID_CONFIG");
  }

  protected static String createProviderPrefix(String providerName)
  {
    return "DBSme.DataProviders." + StringEncoderDecoder.encodeDot(providerName);
  }

  public String getSmeId()
  {
    return smeId;
  }
}
