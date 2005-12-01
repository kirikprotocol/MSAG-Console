package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.resource_group.ResourceGroupConstants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
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
	if (getStatusOnline())
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
    if (!getStatusOnline()) {
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
//		String stopFileName = (String) appContext.getSmsc().getNodeName2Id().get(checkedSmsc);
//		File stopFile = new File(stopFileName);
//		stopFile.createNewFile();
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
			appContext.getSmscList().applyConfig();
			return warning(SMSCErrors.warning.SMSCconfigHasBeenSaved);
		}
		catch (Throwable e)
		{
			logger.error("Couldn't apply config.", e);
			return error(SMSCErrors.error.smsc.couldntApply, e);
		}
	}

	public boolean getStatusOnline()
	{
		try {
		  return hostsManager.getServiceInfo(Constants.SMSC_SME_ID).isOnline();
		} catch (AdminException e) {
		  return false;
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
