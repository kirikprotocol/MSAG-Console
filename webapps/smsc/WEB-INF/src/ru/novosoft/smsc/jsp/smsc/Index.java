/*
 * Created by igork
 * Date: 13.11.2002
 * Time: 16:34:24
 */
package ru.novosoft.smsc.jsp.smsc;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.Statuses;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

public class Index extends SmscBean
{
	Statuses statuses = null;

	protected String mbRoutesApply = null;
	protected String mbRoutesReset = null;
	protected String mbSubjectsApply = null;
	protected String mbSubjectsReset = null;
	protected String mbAliasesApply = null;
	protected String mbAliasesReset = null;
	protected String mbProfilesApply = null;
	protected String mbProfilesReset = null;
	protected String mbHostsApply = null;
	protected String mbHostsReset = null;
	protected String mbServicesApply = null;
	protected String mbServicesReset = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		statuses = appContext.getStatuses();
		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		if (mbRoutesApply != null)
			return applyRoutes();
		else if (mbSubjectsApply != null)
			return applySubjects();
		else if (mbAliasesApply != null)
			return applyAliases();
		else if (mbProfilesApply != null)
			return applyProfiles();
		else if (mbHostsApply != null)
			return applyHosts();
		else if (mbServicesApply != null)
			return applyServices();
		else
			return RESULT_OK;
	}

	private int applyRoutes()
	{
		return routesSubjectsServices();
	}

	private int applySubjects()
	{
		return routesSubjectsServices();
	}

	private int applyAliases()
	{
		try
		{
			smsc.applyAliases();
			statuses.setAliasesChanged(false);

			// !!!
			statuses.setProfilesChanged(false);
			return RESULT_OK;
		}
		catch (Throwable t)
		{
			logger.error("Couldn't apply changes", t);
			return error(SMSCErrors.error.couldntApplyChanges, t);
		}
	}

	private int applyProfiles()
	{
		return error("not yet implemented");
	}

	private int applyHosts()
	{
		return routesSubjectsServices();
	}

	private int applyServices()
	{
		return routesSubjectsServices();
	}

	private int routesSubjectsServices()
	{
		try
		{
			smsc.applyRoutes();
			statuses.setRoutesChanged(false);
			statuses.setSubjectsChanged(false);
			statuses.setServicesChanged(false);
			statuses.setHostsChanged(false);

			// !!!
			statuses.setProfilesChanged(false);
			return RESULT_OK;
		}
		catch (Throwable t)
		{
			logger.error("Couldn't apply changes", t);
			return error(SMSCErrors.error.couldntApplyChanges, t);
		}
	}

	public boolean isRoutesChanged()
	{
		return statuses.isRoutesChanged();
	}

	public boolean isSubjectsChanged()
	{
		return statuses.isSubjectsChanged();
	}

	public boolean isAliasesChanged()
	{
		return statuses.isAliasesChanged();
	}

	public boolean isProfilesChanged()
	{
		return statuses.isProfilesChanged();
	}

	public boolean isHostsChanged()
	{
		return statuses.isHostsChanged();
	}

	public boolean isServicesChanged()
	{
		return statuses.isServicesChanged();
	}


	public String getMbRoutesApply()
	{
		return mbRoutesApply;
	}

	public void setMbRoutesApply(String mbRoutesApply)
	{
		this.mbRoutesApply = mbRoutesApply;
	}

	public String getMbRoutesReset()
	{
		return mbRoutesReset;
	}

	public void setMbRoutesReset(String mbRoutesReset)
	{
		this.mbRoutesReset = mbRoutesReset;
	}

	public String getMbSubjectsApply()
	{
		return mbSubjectsApply;
	}

	public void setMbSubjectsApply(String mbSubjectsApply)
	{
		this.mbSubjectsApply = mbSubjectsApply;
	}

	public String getMbSubjectsReset()
	{
		return mbSubjectsReset;
	}

	public void setMbSubjectsReset(String mbSubjectsReset)
	{
		this.mbSubjectsReset = mbSubjectsReset;
	}

	public String getMbAliasesApply()
	{
		return mbAliasesApply;
	}

	public void setMbAliasesApply(String mbAliasesApply)
	{
		this.mbAliasesApply = mbAliasesApply;
	}

	public String getMbAliasesReset()
	{
		return mbAliasesReset;
	}

	public void setMbAliasesReset(String mbAliasesReset)
	{
		this.mbAliasesReset = mbAliasesReset;
	}

	public String getMbProfilesApply()
	{
		return mbProfilesApply;
	}

	public void setMbProfilesApply(String mbProfilesApply)
	{
		this.mbProfilesApply = mbProfilesApply;
	}

	public String getMbProfilesReset()
	{
		return mbProfilesReset;
	}

	public void setMbProfilesReset(String mbProfilesReset)
	{
		this.mbProfilesReset = mbProfilesReset;
	}

	public String getMbHostsApply()
	{
		return mbHostsApply;
	}

	public void setMbHostsApply(String mbHostsApply)
	{
		this.mbHostsApply = mbHostsApply;
	}

	public String getMbHostsReset()
	{
		return mbHostsReset;
	}

	public void setMbHostsReset(String mbHostsReset)
	{
		this.mbHostsReset = mbHostsReset;
	}

	public String getMbServicesApply()
	{
		return mbServicesApply;
	}

	public void setMbServicesApply(String mbServicesApply)
	{
		this.mbServicesApply = mbServicesApply;
	}

	public String getMbServicesReset()
	{
		return mbServicesReset;
	}

	public void setMbServicesReset(String mbServicesReset)
	{
		this.mbServicesReset = mbServicesReset;
	}
}
