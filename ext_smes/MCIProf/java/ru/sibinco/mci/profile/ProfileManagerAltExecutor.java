package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;

import java.text.MessageFormat;
import java.util.Properties;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 14.12.2004
 * Time: 16:59:22
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerAltExecutor extends ProfileManagerState implements Executor
{
  private static Category logger = Category.getInstance(ProfileManagerAltExecutor.class);

  private MessageFormat pageFormat = null;

  protected String valueYes = null;
  protected String valueNo  = null;
  protected String valueBusy    = null;
  protected String valueDetach  = null;
  protected String valueNoreply = null;
  protected String valueAbsent  = null;
  protected String valueUncond  = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      pageFormat = new MessageFormat(profileBundle.getString(Constants.PAGE_INFO_ALT));
      valueYes = profileBundle.getString(Constants.VALUE_YES);
      valueNo  = profileBundle.getString(Constants.VALUE_NO);
      valueBusy    = systemBundle.getString(Constants.VALUE_BUSY);
      valueDetach  = systemBundle.getString(Constants.VALUE_DETACH);
      valueNoreply = systemBundle.getString(Constants.VALUE_NOREPLY);
      valueAbsent  = systemBundle.getString(Constants.VALUE_ABSENT);
      valueUncond  = systemBundle.getString(Constants.VALUE_UNCOND);
    } catch (Exception e) {
      final String err = "Executor init error"; logger.error(err, e);
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
    } else {
      logger.warn("Got stored exception", exc);
      state.removeAttribute(Constants.ATTR_ERROR);
    }

    Message resp = new Message();
    if (exc != null) {
      final String msg = errorFormat.format(new Object[] {getErrorMessage(exc)});
      resp.setMessageString(Transliterator.translit(msg));
      return new ExecutorResponse(new Message[]{resp}, true);
    }

    String activeReasons = (String)state.getAttribute(Constants.ATTR_REASONS);
    if (activeReasons == null)
      throw new ExecutingException("Failed to locate '"+Constants.ATTR_REASONS+"' attribute",
                                   ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    activeReasons = activeReasons.toUpperCase();
    String body = ""; int counter = 0;

    if (activeReasons.indexOf('B') != -1) body += (Integer.toString(++counter)+">"+valueBusy+": "+
              (checkEventMask(info.getEventMask(), ProfileInfo.MASK_BUSY) ? valueYes:valueNo)+"\r\n");
    if (activeReasons.indexOf('D') != -1) body += (Integer.toString(++counter)+">"+valueDetach+": "+
              (checkEventMask(info.getEventMask(), ProfileInfo.MASK_DETACH) ? valueYes:valueNo)+"\r\n");
    if (activeReasons.indexOf('N') != -1) body += (Integer.toString(++counter)+">"+valueNoreply+": "+
              (checkEventMask(info.getEventMask(), ProfileInfo.MASK_NOREPLY) ? valueYes:valueNo)+"\r\n");
    if (activeReasons.indexOf('A') != -1) body += (Integer.toString(++counter)+">"+valueAbsent+": "+
              (checkEventMask(info.getEventMask(), ProfileInfo.MASK_ABSENT) ? valueYes:valueNo)+"\r\n");
    if (activeReasons.indexOf('U') != -1) body += (Integer.toString(++counter)+">"+valueUncond+": "+
              (checkEventMask(info.getEventMask(), ProfileInfo.MASK_UNCOND) ? valueYes:valueNo));

    String prefix = "";
    if (counter == 0) { prefix = errorDenied; body=""; }
    else if (body.endsWith("\r\n")) body = body.substring(0, body.length()-2);
    Object[] args = new Object[] { prefix, body };
    final String msg = pageFormat.format(args);
    resp.setMessageString(Transliterator.translit(msg));
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
