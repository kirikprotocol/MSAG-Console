package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

import java.util.Properties;
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
  private String valueService = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(divertBundle.getString(Constants.PAGE_SET));
      valueService = divertBundle.getString(Constants.VALUE_SERVICE);
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
    final String reason  = (String)state.getAttribute(Constants.ATTR_REASON);
    final String menuOpt = (checkReason(reason)) ? ("3>"+valueService):"";
    final String msg = pageFormat.format(new Object [] {menuOpt});
    resp.setMessageString(Transliterator.translit(msg));
    //logger.debug("RS="+reason+",menu opt="+menuOpt+", msg="+resp.getMessageString());
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
