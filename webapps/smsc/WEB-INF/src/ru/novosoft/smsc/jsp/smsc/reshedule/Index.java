package ru.novosoft.smsc.jsp.smsc.reshedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Aug 22, 2003
 * Time: 2:31:17 PM
 */
public class Index extends SmscBean
{
  public static final int RESULT_ADD = SmscBean.PRIVATE_RESULT + 0;
  public static final int RESULT_EDIT = SmscBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = SmscBean.PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbDelete = null;
  private String mbEdit = null;
  private String mbSave = null;
  private String mbReset = null;
  private String editKey = null;
  private String[] checkedShedules = new String[0];
  private Set checkedShedulesSet = new HashSet();
  private Reshedules reshedules = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try
	{
		reshedules = Reshedules.getInstance(appContext);
		if (reshedules == null) {throw new Throwable();}
    }
	catch (Throwable e)
	{
      logger.error("couldn't instantiate Reshedules", e);
      return error(SMSCErrors.error.smsc.reshedule.couldntGetReshedules, e);
    }

    checkedShedulesSet.addAll(Arrays.asList(checkedShedules));

    if (editKey == null)
      editKey = "";

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    if (mbDelete != null)
      return delete();
    if (mbEdit != null)
      return RESULT_EDIT;
    if (mbSave != null)
      return save();
    if (mbReset != null)
      return reset();

    return result;
  }

  private int delete()
  {
    for (int i = 0; i < checkedShedules.length; i++) {
      reshedules.removeShedule(checkedShedules[i]);
      journalAppend(SubjectTypes.TYPE_schedule, checkedShedules[i], Actions.ACTION_DEL);
      appContext.getStatuses().setScheduleChanged(true);
    }
    return RESULT_DONE;
  }

  private int save()
  {
    try {
      reshedules.save();
    } catch (Throwable e) {
      logger.error("Couldn't save reschedules to config", e);
      return error(SMSCErrors.error.smsc.reshedule.couldntSaveConfig, e);
    }
    return RESULT_DONE;
  }

  private int reset()
  {
	try {
		reshedules.reset();
		return RESULT_DONE;
	}
	catch (Throwable t) {
		logger.error("Couldn't reset reschedules from config", t);
		return error(SMSCErrors.error.smsc.reshedule.couldntResetConfig, t);
	}
  }

  public Collection getReshedules()
  {
	if (reshedules != null) return reshedules.getShedules();
	else return null;  
  }

  public Collection getResheduleErrors(String reshedule)
  {
    try {
      List errs = reshedules.getErrCodes(reshedule);
      Collections.sort(errs, new Comparator()
      {
        public int compare(Object o1, Object o2)
        {
          if (o1 instanceof String && o2 instanceof String) {
            String s1 = (String) o1;
            String s2 = (String) o2;
            try {
              return Integer.decode(s1).compareTo(Integer.decode(s2));
            } catch (NumberFormatException e) {
              return 0;
            }
          }
          else
            return 0;
        }
      });
      return errs;
    } catch (AdminException e) {
      logger.error("Couldn't get error codes for shedule \"" + reshedule + "\"");
      return new LinkedList();
    }
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getEditKey()
  {
    return editKey;
  }

  public void setEditKey(String editKey)
  {
    this.editKey = editKey;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getErrorString(String errorCode)
  {
    return reshedules.getErrorString(errorCode);
  }

  public String getMbReset()
  {
    return mbReset;
  }

  public void setMbReset(String mbReset)
  {
    this.mbReset = mbReset;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String[] getCheckedShedules()
  {
    return checkedShedules;
  }

  public void setCheckedShedules(String[] checkedShedules)
  {
    this.checkedShedules = checkedShedules;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public boolean isSheduleChecked(String shedule)
  {
    return checkedShedulesSet.contains(shedule);
  }

  public boolean isAllErrorsAssigned()
  {
	if (reshedules != null) return reshedules.isAllErrorsAssigned();
	else return false;
  }

  public String getDefaultReshedule()
  {
	if (reshedules != null) return reshedules.getDefaultReshedule();
	else return null;
  }

  public String getDefaultResheduleName()
  {
    return Reshedules.DEFAULT_RESHEDULE_NAME;
  }

  public Set getAllErrorCodes() throws AdminException
  {
    return reshedules.getAllErrorCodes();
  }
}
