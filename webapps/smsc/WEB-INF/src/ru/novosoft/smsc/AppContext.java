package ru.novosoft.smsc;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.config.Config;


public interface AppContext
{
  public HttpSession getSession(HttpServletRequest req, HttpServletResponse res);

  public Config getConfig();

  public ServiceManager getServiceManager();
}

