/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:14:48
 */
package ru.novosoft.smsc.jsp;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.util.ArrayList;
import java.util.List;

public abstract class PageBean
{
	public static final int RESULT_OK = 0;
	public static final int RESULT_DONE = 1;
	public static final int RESULT_ERROR = 2;
	protected static final int PRIVATE_RESULT = 3;

	protected Category logger = Category.getInstance(this.getClass());

	protected List errors = null;
	protected SMSCAppContext appContext = null;
	protected ServiceManager serviceManager = null;


	public SMSCAppContext getAppContext()
	{
		return appContext;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		this.errors = errors;
		if (errors == null)
		{
			errors = new ArrayList();
			return error(SMSCErrors.error.errorListNotInitialized);
		}

		if (appContext == null)
			return error(SMSCErrors.error.appContextNotInitialized);

		this.appContext = appContext;
		return init(errors);
	}

	protected int init(List errors)
	{
		serviceManager = appContext.getServiceManager();
		if (serviceManager == null)
			return error(SMSCErrors.error.serviceManagerNotInitialized);
		else
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
}
