package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioStateProcessor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ProcessingException;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 16:03:37
 * To change this template use File | Settings | File Templates.
 */
public class DivertSetPreprocessor extends DivertManagerState implements ScenarioStateProcessor
{
  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    String msg = state.getMessageString();
    if (msg == null)
      throw new ProcessingException("Diver option is undefined", -1);
    msg = msg.trim();
    if (msg.equals("1"))      state.setAttribute(Constants.ATTR_REASON, DivertInfo.BUSY);
    else if (msg.equals("2")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.ABSENT);
    else if (msg.equals("3")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.NOTAVAIL);
    else if (msg.equals("4")) state.setAttribute(Constants.ATTR_REASON, DivertInfo.UNCOND);
    else
      throw new ProcessingException("Diver option '"+msg+"' is unknown", -2);
  }

}
