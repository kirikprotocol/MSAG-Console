package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:17:07
 */
public class ProfileSetExecutor extends ProfileManagerState implements Executor
{
  private static Category logger = Category.getInstance(ProfileSetExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueInform = null;
  protected String valueNotify = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(profileBundle.getString(Constants.PAGE_SET));
      valueInform = profileBundle.getString(Constants.VALUE_INFORM);
      valueNotify = profileBundle.getString(Constants.VALUE_NOTIFY);
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    ProfileManagerException exc = (ProfileManagerException)state.getAttribute(Constants.ATTR_ERROR);
    if (exc != null) {
      logger.warn("Got stored exception", exc);
      state.removeAttribute(Constants.ATTR_ERROR);
      final String msg = errorFormat.format(new Object [] {getErrorMessage(exc)});
      return new ExecutorResponse(translit(msg), false);
    }

    final String reason = (String)state.getAttribute(Constants.ATTR_REASON);
    boolean inform = true;
    if      (reason.equals(Constants.INFORM)) inform = true;
    else if (reason.equals(Constants.NOTIFY)) inform = false;
    else throw new ExecutingException("Profile option '"+reason+"' is unknown",
                                      ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    Map formats = new HashMap(10);
    String formatAlts = ""; int counter = 1;
    for (Iterator i = profileManager.getFormatAlts(inform); i.hasNext(); counter++) {
      FormatType alt = (FormatType)i.next();
      formats.put(new Integer(counter), alt);
      formatAlts += ""+counter+">"+alt.getName();
      if (i.hasNext()) formatAlts += "\n";
    }
    state.removeAttribute(Constants.ATTR_FORMATS);
    state.setAttribute(Constants.ATTR_FORMATS, formats);

    Object args[] = new Object[] {inform ? valueInform:valueNotify, formatAlts};
    final String msg = pageFormat.format(args);
    return new ExecutorResponse(translit(msg), false);
  }

}
