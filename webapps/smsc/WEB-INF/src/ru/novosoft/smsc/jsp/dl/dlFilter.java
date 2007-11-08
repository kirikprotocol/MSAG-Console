package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.util.regex.*;

/**
 * Created by igork
 * Date: 13.03.2003
 * Time: 21:26:27
 */
public class dlFilter extends PageBean
{
  private String[] names = null;
  private String[] owners = null;

  private String mbCancel = null;
  private String mbApply = null;
  private String mbClear = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (names == null) {
      names = preferences.getDlFilter().getNames();
      owners = preferences.getDlFilter().getOwners();
    }

    names = Functions.trimStrings(names);
    owners = Functions.trimStrings(owners);

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbApply != null)
      return apply();
    if (mbClear != null)
      return clear();

    return result;
  }

  private int clear()
  {
    names = new String[0];
    owners = new String[0];
    return RESULT_OK;
  }

  private boolean isStringArrayContainsNonAlphanumeric(String[] strings)
  {
    for (int i = 0; i < strings.length; i++) {
      String str = strings[i];
      for (int j = 0; j < str.length(); j++) {
        final char ch = str.charAt(j);
        if (!Character.isLetterOrDigit(ch) && !Character.isSpaceChar(ch) && ch != '+' && ch != '/')
          return true;
      }
    }
    return false;
  }

  private boolean isStringsAllowed(String cat, String[] strings)
  {
    for (int i = 0; i < strings.length; i++) {
      String str = strings[i];
      for (int j = 0; j < str.length(); j++) {
        final char ch = str.charAt(j);
        if (!Character.isLetterOrDigit(ch) && !Character.isSpaceChar(ch) && ch != '+' && ch != '/') {
          logger.error(cat + " pattern \"" + str + "\" is invalid");
          error(SMSCErrors.error.dl.patternIsInvalid, str);
          return false;
        }
      }
    }
    return true;
  }

  private int apply()
  {
    boolean r1 = isStringsAllowed("Name", names);
    boolean r2 = isStringsAllowed("Owner", owners);
    if (r1 && r2) {
      preferences.getDlFilter().setNames(names);
      preferences.getDlFilter().setOwners(owners);
      return RESULT_DONE;
    } else
      return RESULT_ERROR;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbClear()
  {
    return mbClear;
  }

  public void setMbClear(String mbClear)
  {
    this.mbClear = mbClear;
  }

  public String[] getNames()
  {
    return names;
  }

  public void setNames(String[] names)
  {
    this.names = names;
  }

  public String[] getOwners()
  {
    return owners;
  }

  public void setOwners(String[] owners)
  {
    this.owners = owners;
  }
}
