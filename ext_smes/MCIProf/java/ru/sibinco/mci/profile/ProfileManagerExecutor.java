package ru.sibinco.mci.profile;

import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.mci.Constants;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:16:32
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

    if (exc != null) {
      final String msg = errorFormat.format(new Object[] {getErrorMessage(exc)});
      return new ExecutorResponse(translit(msg), true);
    }
    Object[] args = new Object[] {info.inform ? valueYes:valueNo, info.informFormat.getName(),
                                  info.notify ? valueYes:valueNo, info.notifyFormat.getName()};
    final String msg = pageFormat.format(args);
   return new ExecutorResponse(translit(msg), false);
  }
}
