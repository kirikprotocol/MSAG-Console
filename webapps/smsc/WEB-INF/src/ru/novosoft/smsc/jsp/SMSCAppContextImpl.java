package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;
import ru.novosoft.util.jsp.AppContextImpl;


public class SMSCAppContextImpl extends AppContextImpl implements SMSCAppContext
{
  ConfigManager configManager = null;
  ServiceManager serviceManager = null;
  Smsc smsc = null;

  public SMSCAppContextImpl(String configFileName)
  {
    super();
    try
    {
      org.apache.log4j.BasicConfigurator.configure();

      ConfigManager.Init(configFileName);
      configManager = ConfigManager.getInstance();
      smsc = new Smsc(configManager);
      serviceManager.init(configManager, smsc);
      serviceManager = ServiceManager.getInstance();
    }
    catch (Exception e)
    {
      System.out.println("Exception in initialization:");
      e.printStackTrace();
    }
  }

  public Config getConfig()
  {
    return configManager.getConfig();
  }

  public ServiceManager getServiceManager()
  {
    return serviceManager;
  }

  public Smsc getSmsc()
  {
    return smsc;
  }
}

