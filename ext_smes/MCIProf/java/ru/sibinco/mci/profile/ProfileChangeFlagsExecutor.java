package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.aurorisoft.smpp.Message;

import java.text.MessageFormat;
import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 16:39:16
 * To change this template use File | Settings | File Templates.
 */
public class ProfileChangeFlagsExecutor extends ProfileAbstractExecutor
{
  private static Category logger = Category.getInstance(ProfileChangeFlagsExecutor.class);

  private MessageFormat messageFormat = null;

  private String valueGranted       = null;
  private String valueDenied        = null;
  private String valueMessages      = null;
  private String valueNotifications = null;
  private String valueForAbonent    = null;
  private String valueForCallers    = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      messageFormat = new MessageFormat(profileBundle.getString(ProfileScenarioConstants.TAG_FLAGS));
      valueGranted       = profileBundle.getString(ProfileScenarioConstants.VALUE_GRANTED);
      valueDenied        = profileBundle.getString(ProfileScenarioConstants.VALUE_DENIED);
      valueMessages      = profileBundle.getString(ProfileScenarioConstants.VALUE_MESSAGES);
      valueNotifications = profileBundle.getString(ProfileScenarioConstants.VALUE_NOTIFICATIONS);
      valueForAbonent    = profileBundle.getString(ProfileScenarioConstants.VALUE_FOR_ABONENT);
      valueForCallers    = profileBundle.getString(ProfileScenarioConstants.VALUE_FOR_CALLERS);
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  // Get inform/notify option & switch profile flag
  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    String option = (String)state.getAttribute(ProfileScenarioConstants.ATTR_TYPE);
    if (option == null) throw new ExecutingException("Profile option is undefined", -1);
    ProfileInfo info = getProfileInfo(state);
    Object[] args = null;
    if (option.equalsIgnoreCase(ProfileScenarioConstants.INFORM)) {
      info.inform = !info.inform;
      args = new Object[] {info.inform ? valueGranted:valueDenied, valueMessages, valueForAbonent};
    } else if (option.equalsIgnoreCase(ProfileScenarioConstants.NOTIFY)) {
      info.notify = !info.notify;
      args = new Object[] {info.notify ? valueGranted:valueDenied, valueNotifications, valueForCallers};
    } else {
      throw new ExecutingException("Profile option is unknown", -1);
    }
    setProfileInfo(state, info);

    String msgresp = messageFormat.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
