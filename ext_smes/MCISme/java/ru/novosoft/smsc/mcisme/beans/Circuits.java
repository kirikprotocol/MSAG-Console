package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.Map;
import java.util.Iterator;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.02.2005
 * Time: 15:22:41
 * To change this template use File | Settings | File Templates.
 */
public class Circuits extends MCISmeBean
{
  public static final String CIRCUITS_SECTION_NAME = "MCISme.Circuits";

  private String circuits_new_msc = null;
  private String circuits_new_hsn = null;
  private String circuits_new_spn = null;
  private String circuits_new_tsm = null;

  private String mbDone = null;
  private String mbCancel = null;

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (mbCancel != null)    return RESULT_DONE;
    else if (mbDone != null) return done(request.getParameterMap());

    return result;
  }

  private void restore()
  {
    circuits_new_msc = null; circuits_new_hsn = null;
    circuits_new_spn = null; circuits_new_tsm = null;
    restoreConfig();
  }

  private int done(Map requestParams)
  {
    final String PREFIX = CIRCUITS_SECTION_NAME + '.';
    getConfig().removeSection(CIRCUITS_SECTION_NAME);
    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();)
    {
      String paramName = (String) i.next();
      if (paramName.startsWith(PREFIX) && !paramName.endsWith("msc"))
      {
        final String paramValue = getParamValue(requestParams.get(paramName));
        if (paramValue == null) continue;
        if (paramName.endsWith("tsm")) {
          try { Integer.parseInt(paramValue, 16); }
          catch(NumberFormatException e) {
            restore(); return error("mcisme.error.invalid_hex", paramValue, e);
          }
          getConfig().setString(paramName, paramValue);
        } else {
          int intValue = 0;
          try { intValue = Integer.parseInt(paramValue); }
          catch(NumberFormatException e) {
            restore(); return error("mcisme.error.invalid_int", paramValue, e);
          }
          getConfig().setInt(paramName, intValue);
        }
      }
    }
    if (circuits_new_msc != null && circuits_new_msc.length() > 0 &&
        circuits_new_hsn != null && circuits_new_hsn.length() > 0 &&
        circuits_new_spn != null && circuits_new_spn.length() > 0 &&
        circuits_new_tsm != null && circuits_new_tsm.length() > 0)
    {
      int intValue = 0;
      try { intValue = Integer.parseInt(circuits_new_hsn); }
      catch(NumberFormatException e) {
        restore(); return error("mcisme.error.invalid_hsn", circuits_new_hsn, e);
      }
      getConfig().setInt   (PREFIX + circuits_new_msc + ".hsn", intValue);
      try { intValue = Integer.parseInt(circuits_new_spn); }
      catch(NumberFormatException e) {
        restore(); return error("mcisme.error.invalid_spn", circuits_new_spn, e);
      }
      getConfig().setInt   (PREFIX + circuits_new_msc + ".spn", intValue);
      try {
        circuits_new_tsm = circuits_new_tsm.trim();
        int tsm_length = circuits_new_tsm.length(); 
        if (tsm_length <= 0 || tsm_length > 8)
          throw new NumberFormatException("TSM length="+tsm_length+", must be from 1 up to 8 simbols");
        Long.parseLong(circuits_new_tsm, 16);
      } catch(Exception e) {
        restore(); return error("mcisme.error.invalid_tsm", circuits_new_tsm, e);
      }
      getConfig().setString(PREFIX + circuits_new_msc + ".tsm", circuits_new_tsm);
    }

    getMCISmeContext().setChangedOptions(true);
    return RESULT_DONE;
  }

  private String getParamValue(Object paramObjectValue)
  {
    if (paramObjectValue instanceof String)
      return (String) paramObjectValue;
    else if (paramObjectValue instanceof String[]) {
      String[] paramValues = (String[]) paramObjectValue;
      StringBuffer result = new StringBuffer();
      for (int i = 0; i < paramValues.length; i++) {
        result.append(paramValues[i]);
      }
      return result.toString();
    } else
      return null;
  }

  public List getCircuitsSectionNames() {
    return new SortedList(getConfig().getSectionChildShortSectionNames(CIRCUITS_SECTION_NAME));
  }

  public int getInt(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getInt(paramName);
  }
  public String getString(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getString(paramName);
  }
  public boolean getBool(String paramName)
      throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    return getConfig().getBool(paramName);
  }

  public String getMbDone() {
    return mbDone;
  }
  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }
  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public String getCircuits_new_msc() {
    return circuits_new_msc;
  }
  public void setCircuits_new_msc(String circuits_new_msc) {
    this.circuits_new_msc = circuits_new_msc;
  }

  public String getCircuits_new_hsn() {
    return circuits_new_hsn;
  }
  public void setCircuits_new_hsn(String circuits_new_hsn) {
    this.circuits_new_hsn = circuits_new_hsn;
  }

  public String getCircuits_new_spn() {
    return circuits_new_spn;
  }
  public void setCircuits_new_spn(String circuits_new_spn) {
    this.circuits_new_spn = circuits_new_spn;
  }

  public String getCircuits_new_tsm() {
    return circuits_new_tsm;
  }
  public void setCircuits_new_tsm(String circuits_new_tsm) {
    this.circuits_new_tsm = circuits_new_tsm;
  }
}
