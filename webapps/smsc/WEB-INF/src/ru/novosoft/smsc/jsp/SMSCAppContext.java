/*
 * Author: igork
 * Date: 29.04.2002
 * Time: 12:50:46
 */
package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.service.HostsManager;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.jsp.AppContext;

import javax.sql.DataSource;
import java.util.Locale;


public interface SMSCAppContext extends AppContext
{
	public Config getConfig();

	public HostsManager getHostsManager();

	public UserManager getUserManager();

	public Smsc getSmsc();

	public DataSource getConnectionPool();

	public UserPreferences getUserPreferences(java.security.Principal loginedPrincipal);

	public Statuses getStatuses();

	public void destroy();

	public String getLocaleString(Locale locale, String key);

	SmeManager getSmeManager();

	RouteSubjectManager getRouteSubjectManager();

	ResourcesManager getResourcesManager();
}
