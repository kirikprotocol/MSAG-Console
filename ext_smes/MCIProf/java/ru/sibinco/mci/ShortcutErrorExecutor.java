package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 23.12.2004
 * Time: 20:21:56
 * To change this template use File | Settings | File Templates.
 */
public class ShortcutErrorExecutor implements Executor
{
  private static Category logger = Category.getInstance(ShortcutErrorExecutor.class);

  private MessageFormat pageShortcutErr = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      ScenarioResourceBundle systemBundle = (ScenarioResourceBundle)properties.get(Constants.BUNDLE_SYSTEM);
      pageShortcutErr  = new MessageFormat(systemBundle.getString(Constants.PAGE_ERR_SHORTCUT));
    } catch (Exception e) {
      throw new ScenarioInitializationException("Init failed", e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();
    final String message = pageShortcutErr.format(new Object[] {});
    resp.setMessageString(Transliterator.translit(message));
    return new ExecutorResponse(resp, true);
  }

}
