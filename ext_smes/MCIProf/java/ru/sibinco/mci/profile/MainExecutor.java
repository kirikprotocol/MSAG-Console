package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:32:45
 * To change this template use File | Settings | File Templates.
 */
public class MainExecutor implements Executor
{
  private static Category logger = Category.getInstance(MainExecutor.class);

  protected ScenarioResourceBundle systemBundle = null;
  private MessageFormat pageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      pageFormat =  new MessageFormat(systemBundle.getString(Constants.PAGE_MAIN));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();
    final String msg = pageFormat.format(new Object[] {});
    resp.setMessageString(Transliterator.translit(msg));
    state.setAttribute(Constants.ATTR_MAIN, Constants.ATTR_MAIN);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
