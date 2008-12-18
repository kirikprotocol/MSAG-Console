package ru.sibinco.mci.divert;

import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.util.Transliterator;
import ru.sibinco.mci.Constants;

import java.util.Properties;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 15:04:25
 */
public class DivertManagerState
{
  //private static Category logger = Category.getInstance(DivertManagerState.class);

  protected ScenarioResourceBundle systemBundle = null;
  protected ScenarioResourceBundle divertBundle = null;
  protected DivertManager divertManager = null;
  protected boolean needTranslit = true;

  protected MessageFormat errorFormat = null;
  protected String errorCommunication = null;
  protected String errorNotAccepted   = null;
  protected String errorConnect       = null;
  protected String errorUnknown       = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      systemBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_SYSTEM);
      divertBundle = (ScenarioResourceBundle) properties.get(Constants.BUNDLE_DIVERT);
      errorFormat = new MessageFormat(systemBundle.getString(Constants.PAGE_ERR));
      errorCommunication = divertBundle.getString(Constants.ERROR_COMMUNICATION);
      errorNotAccepted   = divertBundle.getString(Constants.ERROR_NOT_ACCEPTED);
      errorConnect       = divertBundle.getString(Constants.ERROR_CONNECT);
      errorUnknown       = divertBundle.getString(Constants.ERROR_UNKNOWN);
      String ss = systemBundle.getString(Constants.TRANSLIT);
      if( ss != null && ss.equalsIgnoreCase("off") ) needTranslit = false;
    } catch (Exception e) {
      throw new ScenarioInitializationException("Init failed", e);
    }
    divertManager = DivertManager.getInstance();
  }

  protected String translit(String msg) {
    if( needTranslit ) return Transliterator.translit(msg);
    else return msg;
  }

  protected String getErrorMessage(DivertManagerException exc)
  {
    if (exc.code == DivertManagerException.COMMUNICATION) return errorCommunication;
    if (exc.code == DivertManagerException.NOT_ACCEPTED) return errorNotAccepted;
    if (exc.code == DivertManagerException.CONNECT) return errorConnect;
    return errorUnknown;
  }
  protected DivertInfo getDivertInfo(ScenarioState state) throws DivertManagerException
  {
    DivertInfo info = (DivertInfo)state.getAttribute(Constants.ATTR_DIVERT);
    if (info == null) {
      if( divertManager != null ) info = divertManager.getDivertInfo(state.getAbonent());
      if (info != null) state.setAttribute(Constants.ATTR_DIVERT, info);
    }
    return info;
  }
  protected void setDivertInfo(ScenarioState state, DivertInfo info) throws DivertManagerException
  {
    if( divertManager != null ) divertManager.setDivertInfo(state.getAbonent(), info);
    state.setAttribute(Constants.ATTR_DIVERT, info);
  }
  protected boolean checkReason(String reason)
  {
    return divertManager != null && divertManager.checkReason(reason);
  }
}
