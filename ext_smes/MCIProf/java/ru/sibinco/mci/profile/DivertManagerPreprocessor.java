package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioStateProcessor;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.scenario.ProcessingException;
import ru.sibinco.smpp.appgw.util.Transliterator;

import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:47:46
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerPreprocessor extends DivertManagerState implements ScenarioStateProcessor
{
  private static Category logger = Category.getInstance(DivertManagerPreprocessor.class);

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    logger.info("Started");
    if (state.getAttribute(Constants.ATTR_MAIN) == null)
    {
      String msg = state.getMessageString();
      if (msg != null) {
        msg = msg.trim();
        String reason = (String)state.getAttribute(Constants.ATTR_REASON);
        if (reason != null && !msg.equals(Constants.OPTION_EXIT))
        {
          String value = null;
          if (msg.equals("1")) value = Constants.OFF;
          else if (msg.equals("2")) value = Constants.SERVICE;
          else if (msg.equals("3")) value = Constants.VOICEMAIL;
          else value = Transliterator.translit(msg.trim());

          DivertInfo info = getDivertInfo(state);
          if (reason.equals(DivertInfo.BUSY)) info.busy = value;
          else if (reason.equals(DivertInfo.ABSENT)) info.absent = value;
          else if (reason.equals(DivertInfo.NOTAVAIL)) info.notavail = value;
          else if (reason.equals(DivertInfo.UNCOND)) info.uncond = value;
          setDivertInfo(state, info);
          // TODO: catch possible exception on get & set divert
        }
      }
    }
    state.removeAttribute(Constants.ATTR_MAIN);
    logger.info("Ended");
  }
}
