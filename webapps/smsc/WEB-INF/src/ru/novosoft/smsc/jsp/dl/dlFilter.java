package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;
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

		names = trimStrings(names);
    owners = trimStrings(owners);

		return result;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
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

  private boolean isStringsAllowed(String cat, String[] strings)
  {
    boolean result = true;
    for (int i = 0; i < strings.length; i++) {
      String string = strings[i];
      try {
        Pattern pattern = Pattern.compile(string);
      } catch (PatternSyntaxException e) {
        logger.error(cat + " pattern \"" + string + "\" is invalid", e);
        error(cat  + " pattern is invalid", string, e);
        result = false;
      }
    }
    return result;
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
