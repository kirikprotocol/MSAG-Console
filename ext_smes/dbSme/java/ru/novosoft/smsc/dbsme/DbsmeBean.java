package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.security.Principal;
import java.util.List;
import java.util.Set;

/**
 * Created by igork
 * Date: Jul 10, 2003
 * Time: 6:42:34 PM
 */
public class DbsmeBean extends IndexBean {

	public static final int RESULT_OVERVIEW = IndexBean.PRIVATE_RESULT;
	public static final int RESULT_PARAMS = IndexBean.PRIVATE_RESULT + 1;
	public static final int RESULT_DRIVERS = IndexBean.PRIVATE_RESULT + 2;
	public static final int RESULT_PROVIDERS = IndexBean.PRIVATE_RESULT + 3;
	public static final int RESULT_PROVIDER = IndexBean.PRIVATE_RESULT + 4;
	protected static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 5;

	private String menuSelection = null;
	protected Config config = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		try {
			File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");
			if (!tempConfigFile.exists()) {
				File origConfigFile = new File(WebAppFolders.getServiceFolder(appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getHost(), Constants.DBSME_SME_ID), "conf/config.xml");
				if (!origConfigFile.exists()) {
					logger.error("Couldn't find DBSME config file (" + origConfigFile.getAbsolutePath() + ")");
					return error(DBSmeErrors.error.couldntFindConfig, origConfigFile.getAbsolutePath());
				}
				InputStream in = new BufferedInputStream(new FileInputStream(origConfigFile));
				OutputStream out = new BufferedOutputStream(new FileOutputStream(tempConfigFile));
				for (int readedByte = in.read(); readedByte >= 0; readedByte = in.read())
					out.write(readedByte);
				in.close();
				out.close();
			}

			config = new Config(tempConfigFile);
		} catch (Throwable e) {
			logger.error("Couldn't get DBSME config", e);
			return error(DBSmeErrors.error.couldntGetConfig, e);
		}

		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if ("overview".equalsIgnoreCase(menuSelection))
			return RESULT_OVERVIEW;
		if ("params".equalsIgnoreCase(menuSelection))
			return RESULT_PARAMS;
		if ("drivers".equalsIgnoreCase(menuSelection))
			return RESULT_DRIVERS;
		if ("providers".equalsIgnoreCase(menuSelection))
			return RESULT_PROVIDERS;

		return RESULT_OK;
	}

	public byte getServiceStatus()
	{
		try {
			return appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getStatus();
		} catch (AdminException e) {
			logger.error("Couldn't get DBSME status, nested: " + e.getMessage(), e);
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}

	public Set getParameterNames()
	{
		return config.getParameterNames();
	}

	public int getInt(String paramName)
	{
		try {
			return config.getInt(paramName);
		} catch (Exception e) {
			logger.debug("Couldn't get int parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
			return 0;
		}
	}

	public String getString(String paramName)
	{
		try {
			return config.getString(paramName);
		} catch (Exception e) {
			logger.debug("Couldn't get string parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
			return "";
		}
	}

	public boolean getBool(String paramName)
	{
		try {
			return config.getBool(paramName);
		} catch (Exception e) {
			logger.debug("Couldn't get boolean parameter \"" + paramName + "\", nested: " + e.getMessage(), e);
			return false;
		}
	}

	public Object getParameter(String paramName)
	{
		return config.getParameter(paramName);
	}

	public String getMenuSelection()
	{
		return menuSelection;
	}

	public void setMenuSelection(String menuSelection)
	{
		this.menuSelection = menuSelection;
	}

	public Set getSectionChildSectionNames(String sectionName)
	{
		return config.getSectionChildSectionNames(sectionName);
	}
}
