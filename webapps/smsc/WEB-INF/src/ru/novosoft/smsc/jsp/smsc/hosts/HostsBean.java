/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:45:22
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.daemon.DaemonManager;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

abstract public class HostsBean extends PageBean
{
	DaemonManager daemonManager = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		daemonManager = appContext.getDaemonManager();
		if (daemonManager == null)
			return error(SMSCErrors.error.daemonManagerNotInitialized);
		else
			return RESULT_OK;
	}
}
