package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;

import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 15:04:25
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerState
{
  private static Category logger = Category.getInstance(DivertManagerState.class);

  protected ScenarioResourceBundle divertBundle = null;
  protected DivertManager divertManager = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    divertBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_DIVERT);
    divertManager = DivertManager.getInstance();
  }

  protected DivertInfo getDivertInfo(ScenarioState state)
  {
    DivertInfo info = (DivertInfo)state.getAttribute(Constants.ATTR_DIVERT);
    if (info == null) {
      info = divertManager.getDivertInfo(state.getAbonent());
      if (info != null) state.setAttribute(Constants.ATTR_DIVERT, info);
    }
    return info;
  }
  protected void setDivertInfo(ScenarioState state, DivertInfo info)
  {
    divertManager.setDivertInfo(state.getAbonent(), info);
    state.removeAttribute(Constants.ATTR_DIVERT);
    state.setAttribute(Constants.ATTR_DIVERT, info);
  }

}
