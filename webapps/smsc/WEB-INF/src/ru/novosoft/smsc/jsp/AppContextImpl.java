package ru.novosoft.smsc.jsp;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import ru.novosoft.smsc.AppContext;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.config.ConfigManager;

public class AppContextImpl implements AppContext
{
  ConfigManager configManager = null;
  ServiceManager serviceManager = null;

  public AppContextImpl(String configFileName)
  {
    try {
      org.apache.log4j.BasicConfigurator.configure();

      ConfigManager.Init(configFileName);
      configManager = ConfigManager.getInstance();
      serviceManager.init(configManager);
      serviceManager = ServiceManager.getInstance();
    } catch (Exception e) {
      System.out.println("Exception in initialization:");
      e.printStackTrace();
    }
  }

  public HttpSession getSession(HttpServletRequest req, HttpServletResponse res)
  {
    return req.getSession();
  }

  public Config getConfig()
  {
    return configManager.getConfig();
  }

  public ServiceManager getServiceManager()
  {
    return serviceManager;
  }

}

