package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:32:45
 */
public class MainExecutor extends AbstractExecutor
{
  private static Category logger = Category.getInstance(MainExecutor.class);

  private MessageFormat pageFormat = null;
  private MessageFormat pageFormatAlt = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
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
    Integer strategyInt = (Integer)state.getAttribute(Constants.ATTR_STRATEGY);
    if (strategyInt == null)
      throw new ExecutingException("Strategy is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    int strategy = strategyInt.intValue();
    String msg = null;
    if (strategy == Constants.RELEASE_REDIRECT_STRATEGY ||
        strategy == Constants.RELEASE_REDIRECT_RULES_STRATEGY) msg = pageFormat.format(new Object[] {});
    else if (strategy == Constants.RELEASE_PREFIXED_STRATEGY ||
             strategy == Constants.RELEASE_MIXED_STRATEGY) msg = pageFormatAlt.format(new Object[] {});
    else
      throw new ExecutingException("Strategy '"+strategy+"' is invalid", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    state.setAttribute(Constants.ATTR_MAIN, Constants.ATTR_MAIN);
    return new ExecutorResponse(translit(msg), false);
  }
}
