/*
 * Author: igork
 * Date: 29.04.2002
 * Time: 12:50:46
 */
package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.service.HostsManager;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.util.jsp.AppContext;

import javax.sql.DataSource;
import java.util.Locale;
import java.util.Set;


public interface SMSCAppContext extends AppContext
{
  Config getConfig();

  HostsManager getHostsManager();

  UserManager getUserManager();

  Smsc getSmsc();

  Provider getProvider();

  Category getCategory();

  DataSource getConnectionPool();

  UserPreferences getUserPreferences(java.security.Principal loginedPrincipal);

  Statuses getStatuses();

  void destroy();

  String getLocaleString(Locale locale, String key);

  public Set getLocaleStrings(Locale locale, String prefix);

  SmeManager getSmeManager();

  ProviderManager getProviderManager();

  CategoryManager getCategoryManager();

  RouteSubjectManager getRouteSubjectManager();

  ResourcesManager getResourcesManager();

  Journal getJournal();

  WebXml getWebXmlConfig();

  AclManager getAclManager();
}
