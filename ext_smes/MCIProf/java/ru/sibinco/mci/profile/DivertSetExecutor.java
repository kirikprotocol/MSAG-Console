package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
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
public class DivertSetExecutor extends DivertManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertSetExecutor.class);

  private MessageFormat pageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(Transliterator.translit(divertBundle.getString(Constants.PAGE_SET)));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    DivertManagerException exc = (DivertManagerException)state.getAttribute(Constants.ATTR_ERROR);
    Message resp = new Message();
    if (exc != null) {
      logger.warn("Got stored exception", exc);
      state.removeAttribute(Constants.ATTR_ERROR);
      resp.setMessageString(errorFormat.format(new Object [] {getErrorMessage(exc)}));
      return new ExecutorResponse(new Message[]{resp}, true);
    }
    resp.setMessageString(pageFormat.format(new Object[] {}));
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
