package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.infosme.backend.config.provider.Provider;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Sep 1, 2003
 * Time: 4:52:33 PM
 */
public class ProviderEdit extends InfoSmeBean
{
  private String provider = null;
  private String type = null;
  private int connections = 0;
  private String dbInstance = null;
  private String dbUserName = null;
  private String dbUserPassword = null;
  private boolean watchdog = false;

  private boolean initialized = false;
  private boolean create = false;
  private String oldProvider = null;
  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      if (!create) {
        if (provider == null || provider.length() == 0)
          return error("infosme.error.no_provider");

        try {
          Provider p = getInfoSmeConfig().getProvider(provider);
          type = p.getType();
          connections = p.getConnections();
          dbInstance = p.getDbInstance();
          dbUserName = p.getDbUserName();
          dbUserPassword = p.getDbUserPassword();
          watchdog = p.isWatchdog();
          oldProvider = provider;
        } catch (Exception e) {
          logger.error(e);
          return error("infosme.error.config_param", e);
        }
      }
    }
    if (provider == null) provider = "";
    if (type == null) type = "";
    if (dbInstance == null) dbInstance = "";
    if (dbUserName == null) dbUserName = "";
    if (dbUserPassword == null) dbUserPassword = "";
    if (oldProvider == null) oldProvider = "";

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return done();
    if (mbCancel != null)
      return RESULT_DONE;
    return result;
  }

  protected int done()
  {
    if (provider == null || provider.length() == 0)
      return error("infosme.error.no_provider");
    if (!create) {
      if (!oldProvider.equals(provider)) {
        if (getInfoSmeConfig().containsProvider(provider))
          return error("infosme.error.exist_provider", provider);
      }
    }

    Provider p = new Provider();
    p.setName(provider);
    p.setType(type);
    p.setDbInstance(dbInstance);
    p.setDbUserName(dbUserName);
    p.setDbUserPassword(dbUserPassword);
    p.setWatchdog(watchdog);
    getInfoSmeConfig().addProvider(p);
    return RESULT_DONE;

  }

  public String getProvider()
  {
    return provider;
  }

  public void setProvider(String provider)
  {
    this.provider = provider;
  }

  public String getType()
  {
    return type;
  }

  public void setType(String type)
  {
    this.type = type;
  }

  public int getConnectionsInt()
  {
    return connections;
  }

  public void setConnectionsInt(int connections)
  {
    this.connections = connections;
  }

  public String getConnections()
  {
    return String.valueOf(connections);
  }

  public void setConnections(String connections)
  {
    try {
      this.connections = Integer.decode(connections).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.DataProvider." + StringEncoderDecoder.encodeDot(provider == null ? "" : provider) + ".connections parameter value: \"" + connections + '"', e);
      this.connections = 0;
    }
  }

  public String getDbInstance()
  {
    return dbInstance;
  }

  public void setDbInstance(String dbInstance)
  {
    this.dbInstance = dbInstance;
  }

  public String getDbUserName()
  {
    return dbUserName;
  }

  public void setDbUserName(String dbUserName)
  {
    this.dbUserName = dbUserName;
  }

  public String getDbUserPassword()
  {
    return dbUserPassword;
  }

  public void setDbUserPassword(String dbUserPassword)
  {
    this.dbUserPassword = dbUserPassword;
  }

  public boolean isWatchdog()
  {
    return watchdog;
  }

  public void setWatchdog(boolean watchdog)
  {
    this.watchdog = watchdog;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }

  public String getOldProvider()
  {
    return oldProvider;
  }

  public void setOldProvider(String oldProvider)
  {
    this.oldProvider = oldProvider;
  }
}
