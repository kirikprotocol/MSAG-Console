package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ProcessingException;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 16:29:53
 * To change this template use File | Settings | File Templates.
 */
public class ProfileChangePreprocessor implements ru.sibinco.smpp.appgw.scenario.ScenarioStateProcessor
{
  public void init(Properties properties) throws ScenarioInitializationException {
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    String msg = state.getMessageString();
    String option = ProfileScenarioConstants.getOption(msg.trim());
    if (option == null) {
      throw new ProcessingException("Profile option is unknown for command '"+msg+"'", -1);
    }
    state.removeAttribute(ProfileScenarioConstants.ATTR_TYPE);
    state.setAttribute(ProfileScenarioConstants.ATTR_TYPE, option);
  }

}
