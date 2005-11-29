package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.io.File;

public class Switch extends SmscBean
{
  protected String mbOnline = null;
  protected String mbOffline = null;
  protected String mbStop = null;
  protected String mbApplyConfig = null;
  protected String mbSwitchOver = null;

  protected String checkedSmsc = null;

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;
    if (appContext.getInstallType() != ResourceGroupConstants.RESOURCEGROUP_TYPE_HA) return error(SMSCErrors.error.smsc.contextInitFailed);

    if (checkedSmsc == null) return RESULT_OK;

    if (mbOnline != null)
      return processOnline();
    else if (mbOffline != null)
      return processOffline();
    else if (mbStop != null)
      return processStop();
    else if (mbApplyConfig != null)
      return processApplyConfig();
	else if (mbSwitchOver != null)
	  return processSwitchOver();
    else
      return RESULT_OK;
  }

  private int processSwitchOver()
  {
	try
	{
		appContext.getSmscList().switchSmsc(checkedSmsc);
	}
	catch (Exception e)
	{
		logger.error("Couldn't switch SMSC over", e);
		return error(SMSCErrors.error.smsc.couldntSwitchOver, e);
	}
	return RESULT_OK;
  }

  private int processOffline()
  {
	if (getStatus() != ServiceInfo.STATUS_ONLINE)
	{
		try
		{
			hostsManager.shutdownService(Constants.SMSC_SME_ID);
			return RESULT_OK;
		}
		catch (Throwable e)
		{
			logger.error("Couldn't offline SMSC", e);
			return error(SMSCErrors.error.smsc.couldntStop, e);
		}
	}
		else return RESULT_OK;
  }

  private int processOnline()
  {
    if (getStatus() != ServiceInfo.STATUS_OFFLINE) {
      try {
        hostsManager.startService(Constants.SMSC_SME_ID);
        return RESULT_OK;
      } catch (Throwable e) {
        logger.error("Couldn't start SMSC", e);
        return error(SMSCErrors.error.smsc.couldntStart, e);
      }
    }
    else
      return RESULT_OK;
  }

  private int processStop()
  {
	try
	{
		String stopFileName = appContext.getSmsc().getStopFileName();
		File stopFile = new File(stopFileName);
		stopFile.createNewFile();
		hostsManager.shutdownService(Constants.SMSC_SME_ID);
		return RESULT_OK;
	}
	catch (Throwable e)
	{
		logger.error("Couldn't stop SMSC", e);
		return error(SMSCErrors.error.smsc.couldntStop, e);
	}
  }

	private int processApplyConfig()
	{
		try
		{
			appContext.getSmscList().applyConfig(checkedSmsc);
			return RESULT_OK;
		}
		catch (Throwable e)
		{
			logger.error("Couldn't apply config " + checkedSmsc, e);
			return error(SMSCErrors.error.smsc.couldntApply, e);
		}
	}

	public byte getStatus()
	{
		try
		{
			return hostsManager.getServiceStatus(Constants.SMSC_SME_ID);
		}
		catch (AdminException e)
		{
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}

	public String getMbOffline()
	{
		return mbOffline;
	}

	public void setMbOffline(final String mbOffline)
	{
		this.mbOffline = mbOffline;
	}


	public String getMbOnline()
	{
		return mbOnline;
	}

	public void setMbOnline(final String mbOnline)
	{
		this.mbOnline = mbOnline;
	}

	public String getMbStop()
	{
		return mbStop;
	}

	public void setMbStop(final String mbStop)
	{
		this.mbStop = mbStop;
	}

	public String getMbApplyConfig()
	{
		return mbApplyConfig;
	}

	public void setMbApplyConfig(final String mbApplyConfig)
	{
		this.mbApplyConfig = mbApplyConfig;
	}

	public String getCheckedSmsc()
	{
		return checkedSmsc;
	}

	public void setCheckedSmsc(final String checkedSmsc)
	{
		this.checkedSmsc = checkedSmsc;
	}

	public String getMbSwitchOver()
	{
		return mbSwitchOver;
	}

	public void setMbSwitchOver(final String mbSwitchOver)
	{
		this.mbSwitchOver = mbSwitchOver;
	}
}
