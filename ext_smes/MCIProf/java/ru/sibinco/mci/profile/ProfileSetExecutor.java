package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;
import java.util.Collection;
import java.util.Iterator;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:17:07
 * To change this template use File | Settings | File Templates.
 */
public class ProfileSetExecutor extends ProfileManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertSetExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueInform = null;
  protected String valueNotify = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try
    {
      super.init(properties);
      pageFormat = new MessageFormat(Transliterator.translit(profileBundle.getString(Constants.PAGE_SET)));
      valueInform = Transliterator.translit(profileBundle.getString(Constants.VALUE_INFORM));
      valueNotify = Transliterator.translit(profileBundle.getString(Constants.VALUE_NOTIFY));
    }
    catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    String reason = (String)state.getAttribute(Constants.ATTR_REASON);
    boolean inform = true;
    if      (reason.equals(Constants.INFORM)) inform = true;
    else if (reason.equals(Constants.NOTIFY)) inform = false;
    else throw new ExecutingException("Profile option '"+reason+"' is unknown",
                                      ErrorCode.PAGE_EXECUTOR_EXCEPTION);

    HashMap formats = new HashMap();
    String formatAlts = ""; int counter = 1;
    for (Iterator i = profileManager.getFormatAlts(inform); i.hasNext(); counter++) {
      FormatType alt = (FormatType)i.next();
      formats.put(new Integer(counter), alt);
      formatAlts += ""+counter+">"+alt.getName();
      if (i.hasNext()) formatAlts += "\n";
    }
    state.removeAttribute(Constants.ATTR_FORMATS);
    state.setAttribute(Constants.ATTR_FORMATS, formats);

    Object args[] = new Object[] {inform ? valueInform:valueNotify, Transliterator.translit(formatAlts)};
    String pageResp = pageFormat.format(args);
    Message resp = new Message(); resp.setMessageString(pageResp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
