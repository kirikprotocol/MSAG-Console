package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;
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
    systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
    try {
      pageFormat =  new MessageFormat(Transliterator.translit(systemBundle.getString(Constants.PAGE_MAIN)));
    } catch (ScenarioResourceBundleException e) {
      logger.error("", e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();
    resp.setMessageString(pageFormat.format(new Object[] {}));
    state.setAttribute(Constants.ATTR_MAIN, Constants.ATTR_MAIN);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
