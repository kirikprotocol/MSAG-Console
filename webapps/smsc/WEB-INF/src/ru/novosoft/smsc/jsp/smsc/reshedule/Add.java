package ru.novosoft.smsc.jsp.smsc.reshedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.security.Principal;
import java.util.List;

/**
 * Created by igork
 * Date: Aug 22, 2003
 * Time: 6:20:08 PM
 */
public class Add extends Body {
  private String mbSave = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (getReshedule() == null)
      setReshedule("");

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbSave != null)
      return save();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int save()
  {
    if (isReshedulePresent())
      return error(SMSCErrors.error.smsc.reshedule.resheduleAlreadyPresent, getReshedule());

    try {
      putSheduleToConfig();
    } catch (AdminException e) {
      return error(SMSCErrors.error.smsc.reshedule.couldntSaveConfig, e);
    }
    return RESULT_DONE;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
