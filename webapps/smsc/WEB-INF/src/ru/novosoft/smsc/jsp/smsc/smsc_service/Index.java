/**
 * Created by igork
 * Date: Dec 9, 2002
 * Time: 4:07:15 PM
 */
package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class Index extends SmscBean
{
	public static final int RESULT_ADD = SmscBean.PRIVATE_RESULT;
	//public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = SmscBean.PRIVATE_RESULT + 2;

	private String mbStart = null;
	private String mbStop = null;
	private String mbSave = null;
	private String mbReset = null;
	private Map params = new HashMap();

	public int process(SMSCAppContext appContext, List errors, HttpServletRequest request)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		processParams(appContext, request);

		try
		{
			serviceManager.refreshService(Constants.SMSC_SME_ID);
		}
		catch (AdminException e)
		{
			return error(SMSCErrors.error.smsc.couldntRefreshStatus);
		}

		if (mbStart != null)
			return processStart();
		else if (mbStop != null)
			return processStop();
		else if (mbSave != null)
			return processApply(appContext);
		else if (mbReset != null)
			return processReset(appContext);
		else
			return RESULT_OK;
	}

	private int processApply(SMSCAppContext appContext)
	{
		Config config = appContext.getSmsc().getSmscConfig();
		if (config == null)
			return error(SMSCErrors.error.smsc.couldntGetConfig);

		for (Iterator i = params.keySet().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			Object value = params.get(name);
			if (value instanceof Boolean)
				config.setBool(name, ((Boolean) value).booleanValue());
			else if (value instanceof Integer)
				config.setInt(name, ((Integer) value).intValue());
			else
				config.setString(name, (String) value);
		}
		try
		{
			appContext.getSmsc().applyConfig(config);
		}
		catch (AdminException e)
		{
			logger.error("Couldn't apply new SMSC config", e);
			return error(SMSCErrors.error.smsc.couldntApply);
		}
		return RESULT_OK;
	}

	private int processReset(SMSCAppContext appContext)
	{
		params.clear();
		return loadParams(appContext);
	}

	private int loadParams(SMSCAppContext appContext)
	{
		Config config = appContext.getSmsc().getSmscConfig();
		if (config == null)
			return error(SMSCErrors.error.smsc.couldntGetConfig);

		for (Iterator i = config.getParameterNames().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			params.put(name, config.getParameter(name));
		}
		return RESULT_OK;
	}

	private int processParams(SMSCAppContext appContext, HttpServletRequest request)
	{
		int result = RESULT_OK;
		if (params.size() == 0)
			loadParams(appContext);

		boolean isRequestHaveParams = false;

		Enumeration parameterNames = request.getParameterNames();
		while (parameterNames.hasMoreElements())
		{
			String s = (String) parameterNames.nextElement();
			if (s.indexOf('.') > 0)
			{
				isRequestHaveParams = true;
				Object oldValue = params.get(s);
				if (oldValue != null)
				{
					if (oldValue instanceof Integer)
					{
						String parameter = request.getParameter(s);
						try
						{
							if (parameter != null && parameter.trim().length() > 0)
								params.put(s, Integer.decode(parameter.trim()));
							else
								params.put(s, new Integer(0));
						}
						catch (NumberFormatException e)
						{
							logger.error("Invalid integer parameter: " + s + "=" + parameter);
							result = error(SMSCErrors.error.smsc.invalidIntParameter, s);
						}
					}
					else if (oldValue instanceof Boolean)
						params.put(s, Boolean.valueOf(request.getParameter(s)));
					else
						params.put(s, request.getParameter(s));
				}
				else
					params.put(s, request.getParameter(s));
			}
		}

		if (isRequestHaveParams)
		{
			for (Iterator i = params.keySet().iterator(); i.hasNext();)
			{
				String paramName = (String) i.next();
				Object value = params.get(paramName);
				if (value instanceof Boolean)
				{
					final String parameter = request.getParameter(paramName);
					params.put(paramName, Boolean.valueOf(parameter));
				}
			}
		}
		return result;
	}

	private int processStart()
	{
		if (getStatus() != ServiceInfo.STATUS_RUNNING)
		{
			try
			{
				Daemon smscDaemon = daemonManager.getSmscDaemon();
				if (smscDaemon == null)
					return error(SMSCErrors.error.smsc.daemonNotFound);
				smsc.start(smscDaemon);
				serviceManager.refreshService(Constants.SMSC_SME_ID);
				return RESULT_OK;
			}
			catch (Throwable e)
			{
				logger.error("Couldn't start SMSC", e);
				return error(SMSCErrors.error.smsc.couldntStart, e);
			}
		}
		else
			return RESULT_OK;
	}

	private int processStop()
	{
		if (getStatus() != ServiceInfo.STATUS_STOPPED)
		{
			try
			{
				Daemon smscDaemon = daemonManager.getSmscDaemon();
				if (smscDaemon == null)
					return error(SMSCErrors.error.smsc.daemonNotFound);
				smsc.stop(smscDaemon);
				serviceManager.refreshService(Constants.SMSC_SME_ID);
				return RESULT_OK;
			}
			catch (Throwable e)
			{
				logger.error("Couldn't stop SMSC", e);
				return error(SMSCErrors.error.smsc.couldntStop, e);
			}
		}
		else
			return RESULT_OK;
	}

	public byte getStatus()
	{
		try
		{
			return serviceManager.getServiceInfo(Constants.SMSC_SME_ID).getStatus();
		}
		catch (AdminException e)
		{
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}

	public String getMbStart()
	{
		return mbStart;
	}

	public void setMbStart(String mbStart)
	{
		this.mbStart = mbStart;
	}

	public String getMbStop()
	{
		return mbStop;
	}

	public void setMbStop(String mbStop)
	{
		this.mbStop = mbStop;
	}

	public String getStringParam(String paramName)
	{
		Object param = params.get(paramName);
		if (param == null)
			return "not found " + params.size();
		if (param instanceof String)
			return (String) param;
		else
			return null;
	}

	public int getIntParam(String paramName)
	{
		Object param = params.get(paramName);
		if (param == null)
		{
			logger.error("integer parameter \"" + paramName + "\" not found.");
			return 0;
		}
		if (param instanceof Integer)
		{
			return ((Integer) param).intValue();
		}
		else
		{
			logger.error("parameter \"" + paramName + "\" is not integer.");
			return -1;
		}
	}

	public boolean getBoolParam(String paramName)
	{
		Object param = params.get(paramName);
		if (param == null)
		{
			logger.error("boolean parameter \"" + paramName + "\" not found.");
			return false;
		}
		if (param instanceof Boolean)
		{
			return ((Boolean) param).booleanValue();
		}
		else
		{
			logger.error("parameter \"" + paramName + "\" is not boolean.");
			return false;
		}
	}

	public void setStringParam(String paramName, String paramValue)
	{
		params.put(paramName, paramValue);
	}

	public void setIntParam(String paramName, int paramValue)
	{
		params.put(paramName, new Integer(paramValue));
	}

	public void setBoolParam(String paramName, boolean paramValue)
	{
		params.put(paramName, new Boolean(paramValue));
	}

	public Set getDatasourceDrivers()
	{
		Set result = new HashSet();
		final String prefix = "StartupLoader.DataSourceDrivers.";
		for (Iterator i = params.keySet().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			if (name.startsWith(prefix))
			{
				String s = name.substring(prefix.length());
				result.add(s.substring(0, s.indexOf('.')));
			}
		}
		return result;
	}

	public String getMbSave()
	{
		return mbSave;
	}

	public void setMbSave(String mbSave)
	{
		this.mbSave = mbSave;
	}

	public String getMbReset()
	{
		return mbReset;
	}

	public void setMbReset(String mbReset)
	{
		this.mbReset = mbReset;
	}
}
