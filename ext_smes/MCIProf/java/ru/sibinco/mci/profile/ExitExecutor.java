package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

import java.text.MessageFormat;
import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 22.10.2004
 * Time: 17:44:04
 * To change this template use File | Settings | File Templates.
 */
public class ExitExecutor implements Executor
{
  private static Category logger = Category.getInstance(MainExecutor.class);

  protected ScenarioResourceBundle systemBundle = null;
  private MessageFormat pageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      pageFormat   =  new MessageFormat(systemBundle.getString(Constants.PAGE_EXIT));
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
    return new ExecutorResponse(new Message[]{resp}, true);
  }

}
