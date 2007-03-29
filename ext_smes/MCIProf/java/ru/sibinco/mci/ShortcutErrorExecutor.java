package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.*;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 23.12.2004
 * Time: 20:21:56
*/
public class ShortcutErrorExecutor extends AbstractExecutor
{
  private MessageFormat pageShortcutErr = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
    try {
      pageShortcutErr  = new MessageFormat(systemBundle.getString(Constants.PAGE_ERR_SHORTCUT));
    } catch (Exception e) {
      throw new ScenarioInitializationException("Init failed", e);
    }
  }
  

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    final String message = pageShortcutErr.format(new Object[] {});
    return new ExecutorResponse(translit(message), true);
  }

}
