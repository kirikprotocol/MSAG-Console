package ru.novosoft.smsc.jsp.smsc.reshedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;

/**
 * Created by igork
 * Date: Aug 22, 2003
 * Time: 6:20:24 PM
 */
public class Body extends PageBean {
  private String reshedule = null;
  private String[] checkedErrs = null;
  protected Set checkedErrsSet = new HashSet();
  private Reshedules reshedules = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      reshedules = Reshedules.getInstance(appContext);
    } catch (Throwable e) {
      logger.error("couldn't instantiate Reshedules", e);
      return error(SMSCErrors.error.smsc.reshedule.couldntGetReshedules, e);
    }

    if (checkedErrs == null)
      checkedErrs = new String[0];

    checkedErrsSet = new HashSet(Arrays.asList(checkedErrs));

    return result;
  }

  protected void getErrCodesFromConfig() throws AdminException
  {
    checkedErrsSet.addAll(reshedules.getErrCodes(reshedule));
  }


  protected void putSheduleToConfig() throws AdminException
  {
    reshedules.putShedule(reshedule, checkedErrsSet);
  }

  protected void removeShedule(String sheduleString)
  {
    reshedules.removeShedule(sheduleString);
  }

  public boolean isErrChecked(String errCode)
  {
    return checkedErrsSet.contains(errCode);
  }

  public String getReshedule()
  {
    return reshedule;
  }

  public void setReshedule(String reshedule)
  {
    this.reshedule = reshedule;
  }

  public String[] getCheckedErrs()
  {
    return checkedErrs;
  }

  public void setCheckedErrs(String[] checkedErrs)
  {
    this.checkedErrs = checkedErrs;
  }

  public String[] getAllErrCodes()
  {
    return Reshedules.ERR_CODES_ALL;
  }

  public String getErrorString(Locale locale, String errorCode)
  {
    return reshedules.getErrorString(locale, errorCode);
  }

  protected boolean isReshedulePresent()
  {
    return reshedules.isShedulePresent(reshedule);
  }

  public boolean isErrorAssigned(String errorCode)
  {
    return reshedules.isErrorAssigned(errorCode);
  }
}
