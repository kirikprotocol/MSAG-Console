package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ExecutorResponse;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ExecutingException;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:05:13
 * To change this template use File | Settings | File Templates.
 */
public class ProfileViewExecutor extends ProfileAbstractExecutor
{
  private static Category logger = Category.getInstance(ProfileViewExecutor.class);

  private MessageFormat messageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      messageFormat = new MessageFormat(profileBundle.getString(ProfileScenarioConstants.TAG_INFO));
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    ProfileInfo info = getProfileInfo(state);
    final String informStr = (info.inform) ? valueInform:valueNoInform;
    final String notifyStr = (info.notify) ? valueNotify:valueNoNotify;
    Object[] args  = new Object[] {informStr, notifyStr, info.getInformFormat(), info.getNotifyFormat()};
    String msgresp = messageFormat.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
