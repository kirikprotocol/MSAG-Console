package ru.sibinco.mci.divert;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 23.12.2004
 * Time: 20:13:51
 * To change this template use File | Settings | File Templates.
 */
public class DivertShortcutExecutor extends DivertManagerState implements Executor
{
  private static Category logger = Category.getInstance(DivertShortcutExecutor.class);

  private int shortcut = -1;

  protected String valueOff = null;
  protected String valueService = null;
  protected String valueVoicemail = null;
  protected String valueChanged = null;

  protected String valueAbsent  = null;
  protected String valueBusy    = null;
  protected String valueNoreply = null;
  protected String valueUncond  = null;

  private MessageFormat pageShortcutInfo = null;
  private MessageFormat pageShortcutErr  = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      shortcut = Integer.parseInt(properties.getProperty(Constants.PARAM_SHORTCUT));
      valueOff = divertBundle.getString(Constants.VALUE_OFF);
      valueService = divertBundle.getString(Constants.VALUE_SERVICE);
      valueVoicemail = divertBundle.getString(Constants.VALUE_VOICEMAIL);
      valueChanged = systemBundle.getString(Constants.VALUE_CHANGED);
      valueAbsent  = systemBundle.getString(Constants.VALUE_ABSENT);
      valueBusy    = systemBundle.getString(Constants.VALUE_BUSY);
      valueNoreply = systemBundle.getString(Constants.VALUE_NOREPLY);
      valueUncond  = systemBundle.getString(Constants.VALUE_UNCOND);
      pageShortcutInfo = new MessageFormat(divertBundle.getString(Constants.PAGE_INFO_SHORTCUT));
      pageShortcutErr  = new MessageFormat(divertBundle.getString(Constants.PAGE_ERR_SHORTCUT));
    } catch (Exception e) {
      final String err = "Executor init error";
      logger.error(err, e);
      throw new ScenarioInitializationException(err, e);
    }
  }

  private String getValue(String option)
  {
    if (option == null || option.equalsIgnoreCase(Constants.OFF)) return valueOff;
    else if (option.equalsIgnoreCase(Constants.SERVICE)) return valueService;
    else if (option.equalsIgnoreCase(Constants.VOICEMAIL)) return valueVoicemail;
    return option;
  }
  private String getServiceInfo(ScenarioState state)
  {
    DivertInfo info = null;
    try { info = getDivertInfo(state); } catch(DivertManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }
    Object[] args = new Object[] {"",
                                  getValue(info.getBusy()), getValue(info.getNoreply()),
                                  getValue(info.getNotavail()), getValue(info.getUncond())};
    return pageShortcutInfo.format(args);
  }

  private final static char REASON_BUSY    = 'B';
  private final static char REASON_NOREPLY = 'N';
  private final static char REASON_ABSENT  = 'A';
  private final static char REASON_UNCOND  = 'U';

  private final static String[] OPTION_VALUES = {Constants.OFF, Constants.VOICEMAIL, Constants.SERVICE};

  private String swicthReason(ScenarioState state, char reason, String value) throws ExecutingException
  {
    DivertInfo info = null;
    try
    {
      info = getDivertInfo(state);
      String errorReason = null;
      if      (reason == REASON_BUSY)    { errorReason = valueBusy;    info.setBusy    (value); }
      else if (reason == REASON_NOREPLY) { errorReason = valueNoreply; info.setNoreply (value); }
      else if (reason == REASON_ABSENT)  { errorReason = valueAbsent;  info.setNotavail(value); }
      else if (reason == REASON_UNCOND)  { errorReason = valueUncond;  info.setUncond  (value); }
      else {
        final String err = "Reason '"+reason+"' is not supported";
        logger.error(err);
        throw new ExecutingException(err, ErrorCode.PAGE_EXECUTOR_EXCEPTION);
      }
      if (!checkReason(""+reason)) return pageShortcutErr.format(new Object[] {errorReason});
      setDivertInfo(state, info);
    }
    catch(DivertManagerException exc) {
      return errorFormat.format(new Object[] {getErrorMessage(exc)});
    }
    Object[] args = new Object[] {valueChanged,
                                  getValue(info.getBusy()), getValue(info.getNoreply()),
                                  getValue(info.getNotavail()), getValue(info.getUncond())};
    return pageShortcutInfo.format(args);
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    String message = null;
    switch(shortcut) {
      case 2: // get general info
        message = getServiceInfo(state); break;
      case 20: case 21: case 22: // set busy
        message = swicthReason(state, REASON_BUSY,    OPTION_VALUES[shortcut%20]); break;
      case 30: case 31: case 32: // set noreply
        message = swicthReason(state, REASON_NOREPLY, OPTION_VALUES[shortcut%30]); break;
      case 40: case 41: case 42: // set absent
        message = swicthReason(state, REASON_ABSENT,  OPTION_VALUES[shortcut%40]); break;
      case 50: case 51: case 52: // set uncond
        message = swicthReason(state, REASON_UNCOND,  OPTION_VALUES[shortcut%50]); break;
      default:
        throw new ExecutingException("Shortcut "+shortcut+" is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    }
    Message resp = new Message(); resp.setMessageString(Transliterator.translit(message));
    return new ExecutorResponse(resp, true);
  }

}
