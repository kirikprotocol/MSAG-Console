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
 * Date: 15.03.2005
 * Time: 18:23:49
 * To change this template use File | Settings | File Templates.
 */
public class DetailsExecutor  implements Executor
{
  private static Category logger = Category.getInstance(DetailsExecutor.class);

  private ScenarioResourceBundle detailsBundle = null;

  private String detailsInfo = null;
  private MessageFormat detailsPage = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      String infoName = (String) properties.get("info");
      String pageName = (String) properties.get("page");
      String bundleName = (String) properties.get("bundle");
      detailsBundle = (ScenarioResourceBundle) properties.get(bundleName);
      detailsInfo   = detailsBundle.getString(infoName);
      detailsPage   = new MessageFormat(detailsBundle.getString(pageName));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    Message resp = new Message();
    String page = detailsPage.format(new Object[] {detailsInfo});
    resp.setMessageString(Transliterator.translit(page));
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
