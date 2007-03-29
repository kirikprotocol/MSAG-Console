package ru.sibinco.mci.divert;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.mci.Constants;
import java.util.Properties;
import java.util.Map;
import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:47:46
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
    if (state.getAttribute(Constants.ATTR_MAIN) == null)
    {
      String msg = state.getMessageString();
      if (msg != null) {
        msg = msg.trim();
        String reason = (String)state.getAttribute(Constants.ATTR_REASON);
        Map optsMap = (Map)state.getAttribute(Constants.ATTR_OPTIONS);
        if (reason != null && optsMap != null && !msg.equals(Constants.OPTION_EXIT))
        {
          msg = msg.trim();
          String value = (String)optsMap.get(msg);
          if (value == null) value = translit(msg);

          try {
            DivertInfo info = getDivertInfo(state);
            if      (reason.equals(DivertInfo.BUSY))     info.setBusy(value);
            else if (reason.equals(DivertInfo.NOREPLY))  info.setNoreply(value);
            else if (reason.equals(DivertInfo.NOTAVAIL)) info.setNotavail(value);
            else if (reason.equals(DivertInfo.UNCOND))   info.setUncond(value);
            setDivertInfo(state, info);
            state.removeAttribute(Constants.ATTR_ERROR);
          } catch (DivertManagerException exc) {
            state.setAttribute(Constants.ATTR_ERROR, exc);
            logger.warn("MSC exception was stored in session");
          }
        }
      }
    }
    state.removeAttribute(Constants.ATTR_MAIN);
  }
}
