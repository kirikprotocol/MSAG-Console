/**
 * Created by igork
 * Date: Dec 9, 2002
 * Time: 4:07:15 PM
 */
package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.daemon.Daemon;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import java.util.List;

public class Index extends SmscBean
{
	public static final int RESULT_ADD = SmscBean.PRIVATE_RESULT;
	//public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
	public static final int PRIVATE_RESULT = SmscBean.PRIVATE_RESULT + 2;

	private String mbStart = null;
	private String mbStop = null;

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

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
		else
			return RESULT_OK;
	}

	private int processStart()
	{
		try
		{
			Daemon smscDaemon = daemonManager.getSmscDaemon();
			if (smscDaemon == null)
				return error(SMSCErrors.error.smsc.daemonNotFound);
			smsc.start(smscDaemon);
			return RESULT_OK;
		}
		catch (Throwable e)
		{
			logger.error("Couldn't start SMSC",e);
			return error(SMSCErrors.error.smsc.couldntStart, e.getMessage(), e);
		}
	}
	private int processStop()
	{
		try
		{
			Daemon smscDaemon = daemonManager.getSmscDaemon();
			if (smscDaemon == null)
				return error(SMSCErrors.error.smsc.daemonNotFound);
			smsc.stop(smscDaemon);
			return RESULT_OK;
		}
		catch (Throwable e)
		{
			e.printStackTrace();
			return error(SMSCErrors.error.smsc.couldntStop, e.getMessage(), e);
		}
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
}
