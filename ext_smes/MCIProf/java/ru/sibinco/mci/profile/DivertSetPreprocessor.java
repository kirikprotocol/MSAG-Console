package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;

import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 16:03:37
 * To change this template use File | Settings | File Templates.
 */
public class DivertSetPreprocessor extends DivertManagerState implements ScenarioStateProcessor
{
  private static Category logger = Category.getInstance(DivertSetPreprocessor.class);

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    String msg = state.getMessageString();
    if (msg == null) {
      final String err = "Divert option is undefined";
      logger.error(err);
      throw new ProcessingException(err, ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    }
    msg = msg.trim();
    if      (msg.equals("1")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.BUSY);
    else if (msg.equals("2")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.ABSENT);
    else if (msg.equals("3")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.NOTAVAIL);
    else if (msg.equals("4")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.UNCOND);
    else {
      final String err = "Divert option '"+msg+"' is unknown";
      logger.error(err);
      throw new ProcessingException(err, ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    }
  }
}
