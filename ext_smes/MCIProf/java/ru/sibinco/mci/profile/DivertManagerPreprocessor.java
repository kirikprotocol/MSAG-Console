package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;

import java.util.Properties;
import java.io.IOException;

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

          try {
            DivertInfo info = getDivertInfo(state);
            if (reason.equals(DivertInfo.BUSY))          info.setBusy(value);
            else if (reason.equals(DivertInfo.ABSENT))   info.setAbsent(value);
            else if (reason.equals(DivertInfo.NOTAVAIL)) info.setNotavail(value);
            else if (reason.equals(DivertInfo.UNCOND))   info.setUncond(value);
            setDivertInfo(state, info);
          } catch (IOException e) {
            final String err = "Communication with MSC error";
            logger.error(err, e);
            throw new ProcessingException(err, e, ErrorCode.PAGE_EXECUTOR_EXCEPTION);
          }
        }
      }
    }
    state.removeAttribute(Constants.ATTR_MAIN);
  }
}
