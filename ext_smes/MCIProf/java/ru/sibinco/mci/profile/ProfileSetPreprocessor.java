package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioStateProcessor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ProcessingException;

import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:16:53
 * To change this template use File | Settings | File Templates.
 */
public class ProfileSetPreprocessor extends ProfileManagerState implements ScenarioStateProcessor
{
  private static Category logger = Category.getInstance(ProfileSetPreprocessor.class);

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    String msg = state.getMessageString();
    if (msg == null)
      throw new ProcessingException("Profile option is undefined", -1);
    msg = msg.trim();
    if      (msg.equals("2")) state.setAttribute(Constants.ATTR_REASON, Constants.INFORM);
    else if (msg.equals("4")) state.setAttribute(Constants.ATTR_REASON, Constants.NOTIFY);
    else
      throw new ProcessingException("Profile option '"+msg+"' is unknown", -2);

    state.removeAttribute(Constants.ATTR_FORMATS);
  }

}
