package ru.novosoft.smsc;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;


public interface AppContext
{
  public HttpSession getSession(HttpServletRequest req, HttpServletResponse res);

  public Config getConfig();

  public ServiceManager getServiceManager();
}

