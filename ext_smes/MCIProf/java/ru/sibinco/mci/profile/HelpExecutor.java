package ru.sibinco.mci.profile;

import org.apache.log4j.Category;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;
import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 20:53:53
 * To change this template use File | Settings | File Templates.
 */
public class HelpExecutor implements Executor
{
  private static Category logger = Category.getInstance(HelpExecutor.class);

  protected ScenarioResourceBundle helpBundle = null;

  private String pageInfo = null;
  private String pageManual = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      helpBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_HELP);
      pageInfo   = helpBundle.getString(Constants.PAGE_INFO);
      pageManual = helpBundle.getString(Constants.PAGE_MAN);
    } catch (ScenarioResourceBundleException e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();   resp.setMessageString(Transliterator.translit(pageInfo));
    Message manual = new Message(); manual.setMessageString(pageManual);
    return new ExecutorResponse(new Message[]{resp, manual}, true);
  }

}
