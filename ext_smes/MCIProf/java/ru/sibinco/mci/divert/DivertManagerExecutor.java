package ru.sibinco.mci.divert;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:59:18
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerExecutor extends DivertManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertManagerExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueOff = null;
  protected String valueService = null;
  protected String valueVoicemail = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(divertBundle.getString(Constants.PAGE_INFO));
      valueOff = divertBundle.getString(Constants.VALUE_OFF);
      valueService = divertBundle.getString(Constants.VALUE_SERVICE);
      valueVoicemail = divertBundle.getString(Constants.VALUE_VOICEMAIL);
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  protected String getValue(String option)
  {
    if (option == null || option.equalsIgnoreCase(Constants.OFF)) return valueOff;
    else if (option.equalsIgnoreCase(Constants.SERVICE)) return valueService;
    else if (option.equalsIgnoreCase(Constants.VOICEMAIL)) return valueVoicemail;
    return option;
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    DivertInfo info = null;
    DivertManagerException exc = (DivertManagerException)state.getAttribute(Constants.ATTR_ERROR);
    if (exc == null) {
      try { info = getDivertInfo(state); }
      catch (DivertManagerException e) { exc = e; }
    }
    else {
      logger.warn("Got stored exception", exc);
      state.removeAttribute(Constants.ATTR_ERROR);
    }

    Message resp = new Message();
    if (exc != null) {
      final String msg = errorFormat.format(new Object[] {getErrorMessage(exc)});
      resp.setMessageString(Transliterator.translit(msg));
      return new ExecutorResponse(new Message[]{resp}, true);
    }
    Object[] args = new Object[] {getValue(info.getBusy()), getValue(info.getNoreply()),
                                  getValue(info.getNotavail()), getValue(info.getUncond())};
    final String msg = pageFormat.format(args);
    resp.setMessageString(Transliterator.translit(msg));
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
