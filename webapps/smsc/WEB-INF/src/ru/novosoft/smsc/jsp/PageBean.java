/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:14:48
 */
package ru.novosoft.smsc.jsp;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.service.HostsManager;

import java.util.*;

public abstract class PageBean
{
	public static final int RESULT_OK = 0;
	public static final int RESULT_DONE = 1;
	public static final int RESULT_ERROR = 2;
	protected static final int PRIVATE_RESULT = 3;

	protected Category logger = Category.getInstance(this.getClass());

	protected List errors = null;
	protected SMSCAppContext appContext = null;
	protected HostsManager hostsManager = null;
	public static final int MAX_PRIORITY = 0x7FFF;
	protected java.security.Principal loginedPrincipal = null;
	protected UserPreferences preferences = null;


	public SMSCAppContext getAppContext()
	{
		return appContext;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		this.errors = errors;
		if (errors == null)
		{
			this.errors = new ArrayList();
			return error(SMSCErrors.error.errorListNotInitialized);
		}

		if (appContext == null)
			return error(SMSCErrors.error.appContextNotInitialized);

		this.appContext = appContext;
		this.loginedPrincipal = loginedPrincipal;
		if (this.appContext != null)
			this.preferences = this.appContext.getUserPreferences(this.loginedPrincipal);

		return init(errors);
	}

	protected int init(List errors)
	{
		hostsManager = appContext.getHostsManager();

		if (hostsManager == null)
			return error(SMSCErrors.error.serviceManagerNotInitialized);

		return RESULT_OK;
	}

	protected int error(String errorCode)
	{
		return _error(new SMSCJspException(errorCode));
	}

	protected int error(String errorCode, String param)
	{
		return _error(new SMSCJspException(errorCode, param));
	}

	protected int error(String errorCode, Throwable cause)
	{
		return _error(new SMSCJspException(errorCode, cause));
	}

	protected int error(String errorCode, String param, Throwable cause)
	{
		return _error(new SMSCJspException(errorCode, param, cause));
	}

	protected int _error(SMSCJspException e)
	{
		errors.add(e);
		return RESULT_ERROR;
	}


	protected String[] trimStrings(String[] masks)
	{
		Set newMasks = new HashSet(masks.length);
		for (int i = 0; i < masks.length; i++)
		{
			String mask = masks[i];
			if (mask != null)
			{
				final String m = mask.trim();
				if (m.length() > 0)
					newMasks.add(m);
			}
		}
		String result[] = (String[]) newMasks.toArray(new String[0]);
		Arrays.sort(result);
		return result;
	}
}
