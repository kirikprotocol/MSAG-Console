package ru.sibinco.mci.profile;

import org.apache.log4j.Category;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 15:03:49
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerState
{
  private static Category logger = Category.getInstance(ProfileManagerState.class);

  protected ScenarioResourceBundle profileBundle = null;
  protected ProfileManager profileManager = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    profileBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_PROFILE);
    profileManager = ProfileManager.getInstance();
  }

  protected ProfileInfo getProfileInfo(ScenarioState state)
  {
    ProfileInfo info = (ProfileInfo)state.getAttribute(Constants.ATTR_PROFILE);
    if (info == null) {
      info = profileManager.getProfileInfo(state.getAbonent());
      if (info != null) state.setAttribute(Constants.ATTR_PROFILE, info);
    }
    return info;
  }
  protected void setProfileInfo(ScenarioState state, ProfileInfo info)
  {
    profileManager.setProfileInfo(state.getAbonent(), info);
    state.setAttribute(Constants.ATTR_PROFILE, info);
  }
}
