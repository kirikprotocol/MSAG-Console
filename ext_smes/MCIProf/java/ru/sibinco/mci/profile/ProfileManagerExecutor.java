package ru.sibinco.mci.profile;

import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:16:32
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerExecutor extends ProfileManagerState implements Executor
{
  private static Category logger = Category.getInstance(ProfileManagerExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueYes = null;
  protected String valueNo  = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(profileBundle.getString(Constants.PAGE_INFO));
      valueYes = profileBundle.getString(Constants.VALUE_YES);
      valueNo  = profileBundle.getString(Constants.VALUE_NO);
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    ProfileInfo info = null;
    ProfileManagerException exc = (ProfileManagerException)state.getAttribute(Constants.ATTR_ERROR);
    if (exc == null) {
      try { info = getProfileInfo(state); }
      catch (ProfileManagerException e) { exc = e; }
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
    Object[] args = new Object[] {info.inform ? valueYes:valueNo, info.informFormat.getName(),
                                  info.notify ? valueYes:valueNo, info.notifyFormat.getName()};
    final String msg = pageFormat.format(args);
    resp.setMessageString(Transliterator.translit(msg));
    return new ExecutorResponse(new Message[]{resp}, false);
  }
}
