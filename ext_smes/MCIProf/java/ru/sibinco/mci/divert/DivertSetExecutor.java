package ru.sibinco.mci.divert;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
import java.util.HashMap;
import java.text.MessageFormat;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.09.2004
 * Time: 16:20:11
 */
public class DivertSetExecutor extends DivertManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertSetExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueOffFull   = null;
  protected String valueService   = null;
  protected String valueVoicemail = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(divertBundle.getString(Constants.PAGE_SET));
      valueOffFull = divertBundle.getString(Constants.VALUE_OFF_FULL);
      valueService = divertBundle.getString(Constants.VALUE_SERVICE);
      valueVoicemail = divertBundle.getString(Constants.VALUE_VOICEMAIL);
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    DivertManagerException exc = (DivertManagerException)state.getAttribute(Constants.ATTR_ERROR);
    Message resp = new Message();
    if (exc != null) {
      logger.warn("Got stored exception", exc);
      state.removeAttribute(Constants.ATTR_ERROR);
      final String msg = errorFormat.format(new Object [] {getErrorMessage(exc)});
      resp.setMessageString(Transliterator.translit(msg));
      return new ExecutorResponse(new Message[]{resp}, true);
    }
    DivertInfo info = null;
    try { info = getDivertInfo(state); } catch (DivertManagerException e) {
      final String msg = errorFormat.format(new Object [] {getErrorMessage(e)});
      resp.setMessageString(Transliterator.translit(msg));
      return new ExecutorResponse(new Message[]{resp}, true);
    }

    String reasonValue = null; // show only inactive reasons
    final String reason  = (String)state.getAttribute(Constants.ATTR_REASON);
    if      (reason.equals(DivertInfo.BUSY))     reasonValue = info.getBusy();
    else if (reason.equals(DivertInfo.NOREPLY))  reasonValue = info.getNoreply();
    else if (reason.equals(DivertInfo.NOTAVAIL)) reasonValue = info.getNotavail();
    else if (reason.equals(DivertInfo.UNCOND))   reasonValue = info.getUncond();
    if (reasonValue == null || reasonValue.length() <= 0) reasonValue = Constants.OFF;

    HashMap optsMap = new HashMap();
    state.removeAttribute(Constants.ATTR_OPTIONS);
    String menuOpts = ""; String option = null; int counter = 0;
    if (!reasonValue.equalsIgnoreCase(Constants.OFF)) {
      option = Integer.toString(++counter); menuOpts += option+">"+valueOffFull+"\r\n";
      optsMap.put(option, Constants.OFF);
    }
    if (!reasonValue.equalsIgnoreCase(Constants.VOICEMAIL)) {
      option = Integer.toString(++counter); menuOpts += option+">"+valueVoicemail+"\r\n";
      optsMap.put(option, Constants.VOICEMAIL);
    }
    if (!reasonValue.equalsIgnoreCase(Constants.SERVICE) && checkReason(reason)) {
      option = Integer.toString(++counter); menuOpts += option+">"+valueService+"\r\n";
      optsMap.put(option, Constants.SERVICE);
    }
    state.setAttribute(Constants.ATTR_OPTIONS, optsMap);

    if (menuOpts.endsWith("\r\n")) menuOpts = menuOpts.substring(0, menuOpts.length()-2);
    final String msg = pageFormat.format(new Object [] {menuOpts});
    resp.setMessageString(Transliterator.translit(msg));
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
