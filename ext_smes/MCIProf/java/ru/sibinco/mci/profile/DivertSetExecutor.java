package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.09.2004
 * Time: 16:20:11
 */
public class DivertSetExecutor extends DivertAbstractExecutor
{
  private static Category logger = Category.getInstance(DivertSetExecutor.class);

  private MessageFormat messageOnFormat  = null;
  private MessageFormat messageOffFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      messageOnFormat  = new MessageFormat(divertBundle.getString(DivertScenarioConstants.TAG_SET_ON));
      messageOffFormat = new MessageFormat(divertBundle.getString(DivertScenarioConstants.TAG_SET_OFF));
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    try
    {
      String type = (String)state.getAttribute(DivertScenarioConstants.ATTR_TYPE);
      String msg = state.getMessageString().trim();

      boolean disable = false;
      if (msg.equals("1")) {
        logger.info("Disable divert");
        disable = true;
      } else if (msg.equals("2")) {
        logger.info("Set divert to MissedCalls");
        msg = "MissedCalls";
      } else if (msg.equals("3")) {
        logger.info("Set divert to auto responce");
        msg = "AutoResponce";
      } else {
        logger.info("Set divert to address '"+msg+"'");
      }
      final String reason = divertBundle.getString(DivertScenarioConstants.REASON_PREFIX + type);

      Object args[] = (disable) ? new Object[]{reason}:new Object[]{msg, reason};
      String msgresp = (disable) ? messageOffFormat.format(args):messageOnFormat.format(args);
      Message resp = new Message(); resp.setMessageString(msgresp);
      return new ExecutorResponse(new Message[]{resp}, false);
    }
    catch (Exception e) {
      logger.error("", e);
      throw new ExecutingException(e.getMessage());
    }
  }
}
