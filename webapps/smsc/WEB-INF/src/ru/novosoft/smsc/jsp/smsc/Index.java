/*
 * Created by igork
 * Date: 13.11.2002
 * Time: 16:34:24
 */
package ru.novosoft.smsc.jsp.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.Statuses;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

public class Index extends SmscBean
{
  private static final Set smscSubjectTypes = new HashSet();

  static
  {
    smscSubjectTypes.add(new Byte(SubjectTypes.TYPE_smsc_config));
    smscSubjectTypes.add(new Byte(SubjectTypes.TYPE_schedule));
    smscSubjectTypes.add(new Byte(SubjectTypes.TYPE_directive));
  }


  Statuses statuses = null;

  private String mbApply = null;
  private String[] checks = null;
  private Journal journal;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    statuses = appContext.getStatuses();
    journal = appContext.getJournal();

    if (checks == null)
      checks = new String[0];

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    for (int i = 0; i < checks.length; i++) {
      if (result != RESULT_OK) {
        break;
      }
      String check = checks[i];
      if ("routes".equalsIgnoreCase(check))
        result = applyRoutes();
      else if ("subjects".equalsIgnoreCase(check))
        result = applySubjects();
      else if ("aliases".equalsIgnoreCase(check))
        result = applyAliases();
      else if ("profiles".equalsIgnoreCase(check))
        result = applyProfiles();
      else if ("hosts".equalsIgnoreCase(check))
        result = applyHosts();
//		else if ("services".equalsIgnoreCase(check))
//			result = applyServices();
      else if ("users".equalsIgnoreCase(check))
        result = applyUsers();
      else if ("smsc".equalsIgnoreCase(check))
        result = applySmsc();
    }
    return result;
  }

  private int applySmsc()
  {
    try {
      appContext.getSmsc().applyConfig();
      statuses.setSmscChanged(false);
      journal.clear(smscSubjectTypes);
      return RESULT_OK;
    } catch (AdminException e) {
      logger.error("Couldn't apply new SMSC config", e);
      return error(SMSCErrors.error.smsc.couldntSave);
    }
  }

  private int applyRoutes()
  {
    try {
      smsc.applyRoutes(routeSubjectManager);
      statuses.setRoutesChanged(false);
      statuses.setRoutesSaved(true);
      statuses.setSubjectsChanged(false);
      //statuses.setServicesChanged(false);
      journal.clear(SubjectTypes.TYPE_route);
      journal.clear(SubjectTypes.TYPE_subject);

      return RESULT_OK;
    } catch (Throwable t) {
      logger.error("Couldn't apply changes", t);
      return error(SMSCErrors.error.couldntApplyChanges, t);
    }
  }

  private int applySubjects()
  {
    try {
      smsc.applyRoutes(routeSubjectManager);
      statuses.setRoutesChanged(false);
      statuses.setRoutesSaved(true);
      statuses.setSubjectsChanged(false);
      //statuses.setServicesChanged(false);
      journal.clear(SubjectTypes.TYPE_route);
      journal.clear(SubjectTypes.TYPE_subject);

      return RESULT_OK;
    } catch (Throwable t) {
      logger.error("Couldn't apply changes", t);
      return error(SMSCErrors.error.couldntApplyChanges, t);
    }
  }

  private int applyAliases()
  {
    try {
      smsc.applyAliases();
      statuses.setAliasesChanged(false);
      journal.clear(SubjectTypes.TYPE_alias);

      return RESULT_OK;
    } catch (Throwable t) {
      logger.error("Couldn't apply changes", t);
      return error(SMSCErrors.error.couldntApplyChanges, t);
    }
  }

  private int applyProfiles()
  {
    smsc.applyProfiles();
    statuses.setProfilesChanged(false);
    journal.clear(SubjectTypes.TYPE_profile);

    return RESULT_OK;
  }

  private int applyHosts()
  {
    try {
      hostsManager.applyHosts();
    } catch (Throwable e) {
      logger.debug("Apply hosts: Exception occured: " + e.getMessage(), e);
      return error(SMSCErrors.error.couldntApplyChanges, "Couldn't apply hosts", e);
    }
    statuses.setHostsChanged(false);
    journal.clear(SubjectTypes.TYPE_host);
    return RESULT_OK;
  }

/*
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

*/
  private int applyUsers()
  {
    try {
      if (statuses.isUsersChanged()) {
        appContext.getUserManager().apply();
        statuses.setUsersChanged(false);
        journal.clear(SubjectTypes.TYPE_user);
      }
      if (statuses.isWebXmlChanged()) {
        appContext.getWebXmlConfig().save();
        statuses.setWebXmlChanged(false);
        journal.clear(SubjectTypes.TYPE_securityConstraint);
      }
      return RESULT_OK;
    } catch (Exception e) {
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

/*
	public boolean isServicesChanged()
	{
		return statuses.isServicesChanged();
	}
*/

  public boolean isUsersChanged()
  {
    return statuses.isUsersChanged();
  }

  public boolean isSmscChanged()
  {
    return statuses.isSmscChanged();
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String[] getChecks()
  {
    return checks;
  }

  public void setChecks(String[] checks)
  {
    this.checks = checks;
  }

  public List getJournalRoutes()
  {
    return journal.getActions(SubjectTypes.TYPE_route);
  }

  public List getJournalSubjects()
  {
    return journal.getActions(SubjectTypes.TYPE_subject);
  }

  public List getJournalAliases()
  {
    return journal.getActions(SubjectTypes.TYPE_alias);
  }

  public List getJournalHosts()
  {
    return journal.getActions(SubjectTypes.TYPE_host);
  }

  public List getJournalUsers()
  {
    return journal.getActions(SubjectTypes.TYPE_user);
  }

  public List getJournalSmsc()
  {
    return journal.getActions(smscSubjectTypes);
  }
}
