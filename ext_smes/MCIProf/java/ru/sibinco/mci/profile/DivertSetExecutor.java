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

  private MessageFormat messageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      messageFormat = new MessageFormat(divertBundle.getString(DivertScenarioConstants.TAG_SET));
    } catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    try {
      String type = (String)state.getAttribute(DivertScenarioConstants.ATTR_TYPE);
      String msg = state.getMessageString();
      if (msg.equals("1")) {
        logger.info("Disable divert");
      } else if (msg.equals("2")) {
        logger.info("Set divert to MissedCalls");
      } else if (msg.equals("3")) {
        logger.info("Set divert to auto responce");
      } else {
        logger.info("Set divert to address '"+msg+"'");
      }

      Object args [] = new Object[] {msg, divertBundle.getString(DivertScenarioConstants.REASON_PREFIX + type)};
      String msgresp = messageFormat.format(args);
      Message resp = new Message();
      resp.setMessageString(msgresp);
      return new ExecutorResponse(new Message[]{resp}, false);
    } catch (Exception e) {
      logger.error("", e);
      throw new ExecutingException(e.getMessage());
    }
  }
}
