package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.resources.ScenarioResourceBundle;
import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.scenario.ScenarioState;
import ru.sibinco.smpp.appgw.util.Transliterator;

import java.util.Properties;
import java.io.IOException;
import java.text.MessageFormat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 15:04:25
 * To change this template use File | Settings | File Templates.
 */
public class DivertManagerState
{
  //private static Category logger = Category.getInstance(DivertManagerState.class);

  protected ScenarioResourceBundle systemBundle = null;
  protected ScenarioResourceBundle divertBundle = null;
  protected DivertManager divertManager = null;

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
      errorFormat = new MessageFormat(Transliterator.translit(systemBundle.getString(Constants.PAGE_ERR)));
      errorCommunication = Transliterator.translit(divertBundle.getString(Constants.ERROR_COMMUNICATION));
      errorNotAccepted   = Transliterator.translit(divertBundle.getString(Constants.ERROR_NOT_ACCEPTED));
      errorConnect       = Transliterator.translit(divertBundle.getString(Constants.ERROR_CONNECT));
      errorUnknown       = Transliterator.translit(divertBundle.getString(Constants.ERROR_UNKNOWN));
    } catch (Exception e) {
      throw new ScenarioInitializationException("Init failed", e);
    }
    divertManager = DivertManager.getInstance();
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
      info = divertManager.getDivertInfo(state.getAbonent());
      if (info != null) state.setAttribute(Constants.ATTR_DIVERT, info);
    }
    return info;
  }
  protected void setDivertInfo(ScenarioState state, DivertInfo info) throws DivertManagerException
  {
    divertManager.setDivertInfo(state.getAbonent(), info);
    state.setAttribute(Constants.ATTR_DIVERT, info);
  }

}
