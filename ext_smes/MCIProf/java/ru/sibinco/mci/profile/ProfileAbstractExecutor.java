package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.Executor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;
import org.apache.log4j.Category;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:05:38
 * To change this template use File | Settings | File Templates.
 */
public abstract class ProfileAbstractExecutor implements Executor
{
  private static Category logger = Category.getInstance(ProfileAbstractExecutor.class);

  protected String valueInform    = null;
  protected String valueNoInform  = null;
  protected String valueNotify    = null;
  protected String valueNoNotify  = null;

  protected ScenarioResourceBundle profileBundle = null;
  protected ProfileManager profileManager = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    profileBundle = (ScenarioResourceBundle) properties.get(ProfileScenarioConstants.TAG_PROFILE);
    try
    {
      valueInform   = profileBundle.getString(ProfileScenarioConstants.VALUE_INFORM);
      valueNoInform = profileBundle.getString(ProfileScenarioConstants.VALUE_NO_INFORM);
      valueNotify   = profileBundle.getString(ProfileScenarioConstants.VALUE_NOTIFY);
      valueNoNotify = profileBundle.getString(ProfileScenarioConstants.VALUE_NO_NOTIFY);
    }
    catch (ScenarioResourceBundleException e) {
      logger.error("", e);
    }
    profileManager = ProfileManager.getInstance();
  }

  protected ProfileInfo getProfileInfo(ScenarioState state)
  {
    ProfileInfo info = (ProfileInfo)state.getAttribute(ProfileScenarioConstants.ATTR_PROFILE);
    if (info == null) {
      info = profileManager.getProfileInfo(state.getAbonent());
      if (info != null) state.setAttribute(ProfileScenarioConstants.ATTR_PROFILE, info);
    }
    return info;
  }
  protected boolean setProfileInfo(ScenarioState state, ProfileInfo info)
  {
    if (info == null) return false;
    profileManager.setProfileInfo(state.getAbonent(), info);
    state.removeAttribute(ProfileScenarioConstants.ATTR_PROFILE);
    state.setAttribute(ProfileScenarioConstants.ATTR_PROFILE, info);
    return true;
  }
}
