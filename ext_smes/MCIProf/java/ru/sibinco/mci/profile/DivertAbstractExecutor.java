package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.Executor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundleException;

import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.09.2004
 * Time: 18:18:43
 * To change this template use File | Settings | File Templates.
 */
public abstract class DivertAbstractExecutor implements Executor
{
  private static Category logger = Category.getInstance(DivertAbstractExecutor.class);

  protected String reasonBusyStr     = null;
  protected String reasonAbsentStr   = null;
  protected String reasonNotAvailStr = null;
  protected String reasonUncondStr   = null;

  protected ScenarioResourceBundle divertBundle = null;
  protected DivertManager divertManager = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    divertBundle = (ScenarioResourceBundle) properties.get(DivertScenarioConstants.TAG_DIVERT);
    try {
      reasonBusyStr     = divertBundle.getString(DivertScenarioConstants.REASON_BUSY);
      reasonAbsentStr   = divertBundle.getString(DivertScenarioConstants.REASON_ABSENT);
      reasonNotAvailStr = divertBundle.getString(DivertScenarioConstants.REASON_NOTAVAIL);
      reasonUncondStr   = divertBundle.getString(DivertScenarioConstants.REASON_UNCOND);
    } catch (ScenarioResourceBundleException e) {
      logger.error("", e);
    }
    divertManager = DivertManager.getInstance();
  }

}
