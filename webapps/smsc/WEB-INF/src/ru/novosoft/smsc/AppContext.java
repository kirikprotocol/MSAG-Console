package ru.novosoft.smsc;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.service.ServiceManager;


public interface AppContext
{
  public HttpSession getSession(HttpServletRequest req, HttpServletResponse res);

  public Config getConfig();
  public ServiceManager getServiceManager();
}

