package ru.sibinco.mci;

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
  private MessageFormat pageFormatAlt = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      pageFormat =  new MessageFormat(systemBundle.getString(Constants.PAGE_MAIN));
      pageFormatAlt =  new MessageFormat(systemBundle.getString(Constants.PAGE_MAIN_ALT));
    } catch (Exception e) {
      final String err = "Executor init error"; logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();
    Integer strategyInt = (Integer)state.getAttribute(Constants.ATTR_STRATEGY);
    if (strategyInt == null)
      throw new ExecutingException("Strategy is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    int strategy = strategyInt.intValue();
    String msg = null;
    if (strategy == Constants.RELEASE_REDIRECT_STRATEGY) msg = pageFormat.format(new Object[] {});
    else if (strategy == Constants.RELEASE_PREFIXED_STRATEGY ||
             strategy == Constants.RELEASE_MIXED_STRATEGY) msg = pageFormatAlt.format(new Object[] {});
    else
      throw new ExecutingException("Strategy '"+strategy+"' is invalid", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    resp.setMessageString(Transliterator.translit(msg));
    state.setAttribute(Constants.ATTR_MAIN, Constants.ATTR_MAIN);
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
