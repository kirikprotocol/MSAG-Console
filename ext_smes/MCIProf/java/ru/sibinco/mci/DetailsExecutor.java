package ru.sibinco.mci;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.*;

import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import org.apache.log4j.Category;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 15.03.2005
 * Time: 18:23:49
 */
public class DetailsExecutor  extends AbstractExecutor
{
  private static Category logger = Category.getInstance(DetailsExecutor.class);

  private String detailsInfo = null;
  private MessageFormat detailsPage = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
    try {
      String infoName = (String) properties.get("info");
      String pageName = (String) properties.get("page");
      String bundleName = (String) properties.get("bundle");
      ScenarioResourceBundle detailsBundle = (ScenarioResourceBundle) properties.get(bundleName);
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
    String page = detailsPage.format(new Object[] {detailsInfo});
    return new ExecutorResponse(translit(page), false);
  }

}
