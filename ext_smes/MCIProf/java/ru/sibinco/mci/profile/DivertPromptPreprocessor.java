package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 07.09.2004
 * Time: 13:09:36
 * To change this template use File | Settings | File Templates.
 */
public class DivertPromptPreprocessor implements ru.sibinco.smpp.appgw.scenario.ScenarioStateProcessor
{
  public void init(Properties properties) throws ScenarioInitializationException {
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    String msg = state.getMessageString();
    String reason = DivertScenarioConstants.getReason(msg);
    if (reason == null)
      throw new ProcessingException("Divert reason is unknown for command '"+msg+"'", -1);
    state.removeAttribute(DivertScenarioConstants.ATTR_TYPE);
    state.setAttribute(DivertScenarioConstants.ATTR_TYPE, reason);
  }
}
