package ru.sibinco.mci.profile;

import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ExecutorResponse;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ExecutingException;
import ru.aurorisoft.smpp.Message;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 16:08:09
 * To change this template use File | Settings | File Templates.
 */
public class ProfileChangeExecutor extends ProfileAbstractExecutor
{
  private static Category logger = Category.getInstance(ProfileViewExecutor.class);

  private MessageFormat messageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      messageFormat = new MessageFormat(profileBundle.getString(ProfileScenarioConstants.TAG_CHANGE));
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    ProfileInfo info = getProfileInfo(state);
    final String informStr = (info.inform) ? valueNoInform:valueInform;
    final String notifyStr = (info.notify) ? valueNoNotify:valueNotify;
    Object[] args  = new Object[] {informStr, notifyStr};
    String msgresp = messageFormat.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
