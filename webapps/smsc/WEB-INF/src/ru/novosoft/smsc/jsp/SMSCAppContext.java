/*
 * Author: igork
 * Date: 29.04.2002
 * Time: 12:50:46
 */
package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.util.conpool.NSConnectionPool;
import ru.novosoft.util.jsp.AppContext;


public interface SMSCAppContext extends AppContext
{
	public Config getConfig();

	public ServiceManager getServiceManager();

	public Smsc getSmsc();

	public NSConnectionPool getConnectionPool();
}
