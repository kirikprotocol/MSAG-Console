/*
 * Author: igork
 * Date: 29.04.2002
 * Time: 12:50:46
 */
package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.service.HostsManager;
import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.util.jsp.AppContext;

import javax.sql.DataSource;
import java.util.*;


public interface SMSCAppContext extends AppContext
{
  Config getConfig();

  HostsManager getHostsManager();

  UserManager getUserManager();

  Smsc getSmsc();

  DataSource getConnectionPool();

  UserPreferences getUserPreferences(java.security.Principal loginedPrincipal);

  Statuses getStatuses();

  void destroy();

  String getLocaleString(Locale locale, String key);

  SmeManager getSmeManager();

  RouteSubjectManager getRouteSubjectManager();

  ResourcesManager getResourcesManager();

  Journal getJournal();

  WebXml getWebXmlConfig();
}
