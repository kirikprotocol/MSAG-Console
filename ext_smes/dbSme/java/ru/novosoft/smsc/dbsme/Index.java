package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.File;
import java.security.Principal;
import java.util.List;


/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:02:35 PM
 */
public class Index extends DbsmeBean {
	private String mbApply = null;
	private String mbReset = null;
	private String mbStart = null;
	private String mbStop = null;

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbApply != null)
			return apply();
		if (mbReset != null)
			return reset();
		if (mbStart != null)
			return start();
		if (mbStop != null)
			return stop();

		return result;
	}

	private int stop()
	{
		try {
			appContext.getHostsManager().shutdownService(Constants.DBSME_SME_ID);
		} catch (AdminException e) {
			return error(DBSmeErrors.error.couldntStopDbSme, e);
		}
		return RESULT_DONE;
	}

	private int start()
	{
		try {
			appContext.getHostsManager().startService(Constants.DBSME_SME_ID);
		} catch (AdminException e) {
			return error(DBSmeErrors.error.couldntStartDbSme, e);
		}
		return RESULT_DONE;
	}

	private int reset()
	{
		File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");
		if (tempConfigFile.exists())
			tempConfigFile.delete();
		DbSmeContext.getInstance(appContext).setConfigChanged(false);
		return RESULT_DONE;
	}

	private int apply()
	{
		File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");
		File origConfigFile = null;
		try {
			origConfigFile = new File(WebAppFolders.getServiceFolder(appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getHost(), Constants.DBSME_SME_ID), "conf/config.xml");
		} catch (AdminException e) {
			return error(DBSmeErrors.error.couldntGetOriginalConfig, e);
		}
		if (origConfigFile.exists())
			if (!origConfigFile.delete())
				return error(DBSmeErrors.error.couldntRemoveOldConfig);
		if (!tempConfigFile.renameTo(origConfigFile))
			return error(DBSmeErrors.error.couldntApplyNewConfig);

		DbSmeContext.getInstance(appContext).setConfigChanged(false);
		return RESULT_DONE;
	}

	public boolean isConfigChanged()
	{
		return DbSmeContext.getInstance(appContext).isConfigChanged();
	}

	public String getMbApply()
	{
		return mbApply;
	}

	public void setMbApply(String mbApply)
	{
		this.mbApply = mbApply;
	}

	public String getMbReset()
	{
		return mbReset;
	}

	public void setMbReset(String mbReset)
	{
		this.mbReset = mbReset;
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
