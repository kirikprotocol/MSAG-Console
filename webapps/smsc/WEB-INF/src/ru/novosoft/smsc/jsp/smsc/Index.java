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
	protected String mbUsersApply = null;
	protected String mbUsersReset = null;
	protected String mbSmscApply = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		statuses = appContext.getStatuses();
		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
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
		else if (mbUsersApply != null)
			return applyUsers();
		else if (mbSmscApply != null)
			return applySmsc();
		else
			return RESULT_OK;
	}

	private int applySmsc()
	{
		try
		{
			appContext.getSmsc().applyConfig();
			statuses.setSmscChanged(false);
			return RESULT_OK;
		}
		catch (AdminException e)
		{
			logger.error("Couldn't apply new SMSC config", e);
			return error(SMSCErrors.error.smsc.couldntSave);
		}
	}

	private int applyRoutes()
	{
		try
		{
			smsc.applyRoutes();
			statuses.setRoutesChanged(false);
			statuses.setSubjectsChanged(false);
			statuses.setServicesChanged(false);

			return RESULT_OK;
		}
		catch (Throwable t)
		{
			logger.error("Couldn't apply changes", t);
			return error(SMSCErrors.error.couldntApplyChanges, t);
		}
	}

	private int applySubjects()
	{
		try
		{
			smsc.applyRoutes();
			statuses.setRoutesChanged(false);
			statuses.setSubjectsChanged(false);
			statuses.setServicesChanged(false);

			return RESULT_OK;
		}
		catch (Throwable t)
		{
			logger.error("Couldn't apply changes", t);
			return error(SMSCErrors.error.couldntApplyChanges, t);
		}
	}

	private int applyAliases()
	{
		try
		{
			smsc.applyAliases();
			statuses.setAliasesChanged(false);

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
		smsc.applyProfiles();
		statuses.setProfilesChanged(false);
		return RESULT_OK;
	}

	private int applyHosts()
	{
		statuses.setHostsChanged(false);
		return RESULT_OK;
	}

	private int applyServices()
	{
		try
		{
			smsc.applyServices();
			statuses.setServicesChanged(false);
			statuses.setSubjectsChanged(false);
			statuses.setRoutesChanged(false);

			return RESULT_OK;
		}
		catch (Throwable t)
		{
			logger.error("Couldn't apply changes", t);
			return error(SMSCErrors.error.couldntApplyChanges, t);
		}
	}

	private int applyUsers()
	{
		try
		{
			appContext.getUserManager().apply();
			statuses.setUsersChanged(false);
			return RESULT_OK;
		}
		catch (Exception e)
		{
			logger.error("Couldn't apply users", e);
			return error(SMSCErrors.error.users.couldntApply, e);
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

	public boolean isUsersChanged()
	{
		return statuses.isUsersChanged();
	}

	public boolean isSmscChanged()
	{
		return statuses.isSmscChanged();
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

	public String getMbUsersApply()
	{
		return mbUsersApply;
	}

	public void setMbUsersApply(String mbUsersApply)
	{
		this.mbUsersApply = mbUsersApply;
	}

	public String getMbUsersReset()
	{
		return mbUsersReset;
	}

	public void setMbUsersReset(String mbUsersReset)
	{
		this.mbUsersReset = mbUsersReset;
	}

	public String getMbSmscApply()
	{
		return mbSmscApply;
	}

	public void setMbSmscApply(String mbSmscApply)
	{
		this.mbSmscApply = mbSmscApply;
	}
}
