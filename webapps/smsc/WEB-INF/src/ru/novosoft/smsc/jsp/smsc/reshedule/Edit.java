package ru.novosoft.smsc.jsp.smsc.reshedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by igork
 * Date: Aug 22, 2003
 * Time: 6:20:44 PM
 */
public class Edit extends Body
{
  private String editKey = null;
  private String mbSave = null;
  private String mbCancel = null;
  private String oldShedule = null;

  private boolean initialized = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      setReshedule(editKey);
      oldShedule = editKey;
      try {
        getErrCodesFromConfig();
      } catch (AdminException e) {
        logger.error("Couldn't get error codes for shedule \"" + editKey + '"', e);
        return error(SMSCErrors.error.smsc.reshedule.couldntGetErrorCodes, e);
      }
    }

    if (editKey == null)
      editKey = "";

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
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
    if (!isDefaultReshedule() && !oldShedule.equals(getReshedule()) && isReshedulePresent())
      return error(SMSCErrors.error.smsc.reshedule.resheduleAlreadyPresent, getReshedule());

    try {
      if (!oldShedule.equals(getReshedule()))
        removeShedule(oldShedule);
      putSheduleToConfig();
      journalAppend(SubjectTypes.TYPE_schedule, getReshedule(), Actions.ACTION_MODIFY, "old schedule", oldShedule);
      appContext.getStatuses().setScheduleChanged(true);
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

  public String getEditKey()
  {
    return editKey;
  }

  public void setEditKey(String editKey)
  {
    this.editKey = editKey;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getOldShedule()
  {
    return oldShedule;
  }

  public void setOldShedule(String oldShedule)
  {
    this.oldShedule = oldShedule;
  }
}
