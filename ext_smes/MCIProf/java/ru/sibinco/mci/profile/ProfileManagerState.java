package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.mci.Constants;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 15:03:49
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerState
{
  protected ScenarioResourceBundle systemBundle = null;
  protected ScenarioResourceBundle profileBundle = null;
  protected ProfileManager profileManager = null;

  protected MessageFormat errorFormat = null;
  protected String errorDB      = null;
  protected String errorUnknown = null;
  protected String errorDenied  = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      systemBundle  = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      profileBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_PROFILE);
      errorFormat   = new MessageFormat(systemBundle.getString(Constants.PAGE_ERR));
      errorDB       = profileBundle.getString(Constants.ERROR_DB);
      errorUnknown  = profileBundle.getString(Constants.ERROR_UNKNOWN);
      errorDenied   = profileBundle.getString(Constants.ERROR_DENIED);
    } catch (Exception e) {
      throw new ScenarioInitializationException("Init failed", e);
    }
    profileManager = ProfileManager.getInstance();
  }

  protected boolean checkEventMask(int userMask, int cause) {
    return ((userMask & cause) == cause);
  }
  protected int switchEventMask(int userMask, int cause) {
    return (((userMask & cause) == cause) ? (userMask & ~cause):(userMask | cause)) & ProfileInfo.MASK_ALL;
  }

  protected String getErrorMessage(ProfileManagerException exc)
  {
    if (exc.code == ProfileManagerException.DB_ERROR) return errorDB;
    return errorUnknown;
  }
  protected ProfileInfo getProfileInfo(ScenarioState state) throws ProfileManagerException
  {
    ProfileInfo info = (ProfileInfo)state.getAttribute(Constants.ATTR_PROFILE);
    if (info == null) {
      info = profileManager.getProfileInfo(state.getAbonent());
      if (info != null) state.setAttribute(Constants.ATTR_PROFILE, info);
    }
    return info;
  }
  protected void setProfileInfo(ScenarioState state, ProfileInfo info)  throws ProfileManagerException
  {
    profileManager.setProfileInfo(state.getAbonent(), info);
    state.setAttribute(Constants.ATTR_PROFILE, info);
  }
}
