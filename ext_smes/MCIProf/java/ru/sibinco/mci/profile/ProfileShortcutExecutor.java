package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.util.Properties;
import java.util.Iterator;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 23.12.2004
 * Time: 16:41:19
 * To change this template use File | Settings | File Templates.
 */
public class ProfileShortcutExecutor extends ProfileManagerState implements Executor
{
  private static Category logger = Category.getInstance(ProfileShortcutExecutor.class);

  private int shortcut = -1;

  protected String valueYes = null;
  protected String valueNo  = null;
  protected String valueBusy    = null;
  protected String valueDetach  = null;
  protected String valueAbsent  = null;
  protected String valueNoreply = null;
  protected String valueUncond  = null;
  protected String valueChanged = null;

  private String        detailsInfo = null;
  private MessageFormat pageShortcutErr = null;
  private MessageFormat pageShortcutInfo = null;
  private MessageFormat pageShortcutAltInfo = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);

      String detailsInfoName = properties.getProperty("info");
      detailsInfo  = (detailsInfoName == null) ? null : profileBundle.getString(detailsInfoName);
      shortcut = Integer.parseInt(properties.getProperty(Constants.PARAM_SHORTCUT));

      valueYes = profileBundle.getString(Constants.VALUE_YES);
      valueNo  = profileBundle.getString(Constants.VALUE_NO);
      valueBusy    = systemBundle.getString(Constants.VALUE_BUSY);
      valueDetach  = systemBundle.getString(Constants.VALUE_DETACH);
      valueAbsent  = systemBundle.getString(Constants.VALUE_ABSENT);
      valueNoreply = systemBundle.getString(Constants.VALUE_NOREPLY);
      valueUncond  = systemBundle.getString(Constants.VALUE_UNCOND);
      valueChanged = systemBundle.getString(Constants.VALUE_CHANGED);

      pageShortcutErr  = new MessageFormat(profileBundle.getString(Constants.PAGE_ERR_SHORTCUT));
      pageShortcutInfo = new MessageFormat(profileBundle.getString(Constants.PAGE_INFO_SHORTCUT));
      pageShortcutAltInfo = new MessageFormat(profileBundle.getString(Constants.PAGE_INFO_ALT_SHORTCUT));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  private String getServiceInfo(ScenarioState state)
  {
    ProfileInfo info = null;
    try { info = getProfileInfo(state); } catch(ProfileManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }
    Object[] args = new Object[] {"",
                                  info.inform ? valueYes:valueNo, info.informFormat.getName(),
                                  info.notify ? valueYes:valueNo, info.notifyFormat.getName()};
    return pageShortcutInfo.format(args);
  }

  private String switchFlag(ScenarioState state, boolean inform)
  {
    ProfileInfo info = null;
    try {
      info = getProfileInfo(state);
      if (inform) info.inform = !info.inform;
      else info.notify = !info.notify;
      setProfileInfo(state, info);
    } catch(ProfileManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }

    Object[] args = new Object[] {valueChanged,
                                  info.inform ? valueYes:valueNo, info.informFormat.getName(),
                                  info.notify ? valueYes:valueNo, info.notifyFormat.getName()};
    return pageShortcutInfo.format(args);
  }

  private String switchMessage(ScenarioState state, boolean inform)
  {
    ProfileInfo info = null;
    try {
      info = getProfileInfo(state);
      long currentFormatId = ((inform) ? info.informFormat:info.notifyFormat).getId();
      for (Iterator i = profileManager.getFormatAlts(inform); i.hasNext(); )
      {
        FormatType alt = (FormatType)i.next();
        if (alt != null && alt.getId() == currentFormatId) {
          alt = (FormatType)(i.hasNext() ? i.next() : profileManager.getFormatAlts(inform).next());
          if (inform) info.informFormat = alt;
          else info.notifyFormat = alt;
          break;
        }
      }
      setProfileInfo(state, info);
    } catch(ProfileManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }
    Object[] args = new Object[] {valueChanged,
                                  info.inform ? valueYes:valueNo, info.informFormat.getName(),
                                  info.notify ? valueYes:valueNo, info.notifyFormat.getName()};
    return pageShortcutInfo.format(args);
  }

  private String getActiveReasons(ScenarioState state) throws ExecutingException
  {
    String activeReasons = (String)state.getAttribute(Constants.ATTR_REASONS);
    if (activeReasons == null)
      throw new ExecutingException("Failed to locate '"+Constants.ATTR_REASONS+"' attribute",
                                   ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    return activeReasons.toUpperCase();
  }
  private String formatMessage(String activeReasons, ProfileInfo info)
  {
    String str = "";
    if (activeReasons.indexOf('B') >= 0) str += (valueBusy + ": " +
          (checkEventMask(info.eventMask, ProfileInfo.MASK_BUSY) ? valueYes:valueNo)+"/r/n");
    if (activeReasons.indexOf('D') >= 0) str += (valueDetach + ": " +
          (checkEventMask(info.eventMask, ProfileInfo.MASK_DETACH) ? valueYes:valueNo)+"/r/n");
    if (activeReasons.indexOf('A') >= 0) str += (valueAbsent + ": " +
          (checkEventMask(info.eventMask, ProfileInfo.MASK_ABSENT) ? valueYes:valueNo)+"/r/n");
    if (activeReasons.indexOf('N') >= 0) str += (valueNoreply + ": " +
          (checkEventMask(info.eventMask, ProfileInfo.MASK_NOREPLY) ? valueYes:valueNo)+"/r/n");
    if (activeReasons.indexOf('U') >= 0) str += (valueUncond + ": " +
          (checkEventMask(info.eventMask, ProfileInfo.MASK_UNCOND) ? valueYes:valueNo));
    return (str.endsWith("\r\n") ? str.substring(0, str.length()-2) : str);
  }
  private String getEventMask(ScenarioState state) throws ExecutingException
  {
    ProfileInfo info = null;
    try { info = getProfileInfo(state); } catch(ProfileManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }
    String activeReasons = getActiveReasons(state);
    return pageShortcutAltInfo.format(new Object[] { formatMessage(activeReasons, info) });
  }

  private String switchEventMask(ScenarioState state, int cause) throws ExecutingException
  {
    String errorReason = null; // check allowed reasons
    String activeReasons = getActiveReasons(state);
    if (cause == ProfileInfo.MASK_BUSY    && activeReasons.indexOf('B') < 0) errorReason = valueBusy;
    if (cause == ProfileInfo.MASK_DETACH  && activeReasons.indexOf('D') < 0) errorReason = valueDetach;
    if (cause == ProfileInfo.MASK_NOREPLY && activeReasons.indexOf('N') < 0) errorReason = valueNoreply;
    if (cause == ProfileInfo.MASK_ABSENT  && activeReasons.indexOf('A') < 0) errorReason = valueAbsent;
    if (cause == ProfileInfo.MASK_UNCOND  && activeReasons.indexOf('U') < 0) errorReason = valueUncond;
    if (errorReason != null) return pageShortcutErr.format(new Object[] {errorReason});

    int strategy = getStrategy(state); // must be set by EntryExecutor
    if (strategy == Constants.RELEASE_MIXED_STRATEGY && cause == ProfileInfo.MASK_ABSENT) {
      if (detailsInfo != null) return detailsInfo;
      logger.warn("Mixed strategy configuration invalid !");
      return errorFormat.format(new Object[] {errorUnknown});
    }

    ProfileInfo info = null;
    try {
      info = getProfileInfo(state);
      int oldEventMask = info.getEventMask();
      info.eventMask = switchEventMask(oldEventMask, cause);
      if (info.eventMask != oldEventMask) setProfileInfo(state, info);
      else logger.warn("Event mask is kept unchanged (mask="+oldEventMask+", cause='"+cause+"')");
    } catch(ProfileManagerException exc) {
      return errorFormat.format(new Object[] { getErrorMessage(exc) });
    }
    return pageShortcutAltInfo.format(new Object[] { formatMessage(activeReasons, info) });
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    String message = null;
    switch(shortcut) {
      case 1:  message = getServiceInfo(state); break;
      case 2:  message = getEventMask(state);   break;
      case 10: message = switchFlag(state, true);     break;
      case 11: message = switchMessage(state, true);  break;
      case 12: message = switchFlag(state, false);    break;
      case 13: message = switchMessage(state, false); break;
      case 20: message = switchEventMask(state, ProfileInfo.MASK_BUSY);    break;
      case 21: message = switchEventMask(state, ProfileInfo.MASK_DETACH);  break;
      case 22: message = switchEventMask(state, ProfileInfo.MASK_NOREPLY); break;
      case 23: message = switchEventMask(state, ProfileInfo.MASK_ABSENT);  break;
      case 24: message = switchEventMask(state, ProfileInfo.MASK_UNCOND);  break;
      default:
        throw new ExecutingException("Shortcut "+shortcut+" is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    }
    Message resp = new Message(); resp.setMessageString(Transliterator.translit(message));
    return new ExecutorResponse(resp, true);
  }

}
