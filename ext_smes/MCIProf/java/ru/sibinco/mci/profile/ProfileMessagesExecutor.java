package ru.sibinco.mci.profile;

import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;
import java.util.Collection;
import java.util.Iterator;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ExecutorResponse;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ExecutingException;
import ru.aurorisoft.smpp.Message;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 18:16:42
 * To change this template use File | Settings | File Templates.
 */
public class ProfileMessagesExecutor extends ProfileAbstractExecutor
{
  private static Category logger = Category.getInstance(ProfileMessagesExecutor.class);

  private MessageFormat messageFormat = null;

  private String valueMessages      = null;
  private String valueNotifications = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      messageFormat = new MessageFormat(profileBundle.getString(ProfileScenarioConstants.TAG_SELECT_FORMAT));
      valueMessages      = profileBundle.getString(ProfileScenarioConstants.VALUE_MESSAGES);
      valueNotifications = profileBundle.getString(ProfileScenarioConstants.VALUE_NOTIFICATIONS);
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    String option = (String)state.getAttribute(ProfileScenarioConstants.ATTR_TYPE);
    if (option == null) throw new ExecutingException("Profile option is undefined", -1);
    boolean inform = true;
    if (option.equalsIgnoreCase(ProfileScenarioConstants.INFORM))      inform = true;
    else if (option.equalsIgnoreCase(ProfileScenarioConstants.NOTIFY)) inform = false;
    else throw new ExecutingException("Profile option is unknown", -1);

    // TODO: mapping formatId <-> optionSelected
    //ProfileInfo info = getProfileInfo(state);
    String formatAlts = ""; int counter = 0;
    Collection alts = profileManager.getFormatAlts(inform);
    for (Iterator i=alts.iterator(); i.hasNext(); ) {
      Object obj = i.next();
      if (obj == null || obj instanceof ServiceMessageType) continue;
      String altName = ((ServiceMessageType)obj).getName();
      formatAlts += ""+(++counter)+"> "+altName;
      if (i.hasNext()) formatAlts += "\n";
      // TODO: possible skip current format
    }

    Object[] args  = new Object[] {inform ? valueMessages:valueNotifications, formatAlts};
    String msgresp = messageFormat.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
