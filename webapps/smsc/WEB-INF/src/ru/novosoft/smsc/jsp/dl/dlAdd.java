package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.admin.dl.*;
import ru.novosoft.smsc.admin.dl.exceptions.ListAlreadyExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalAlreadyExistsException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.*;

/**
 * Created by igork
 * Date: 26.02.2003
 * Time: 18:36:10
 */
public class dlAdd extends dlBody
{
	private String mbSave = null;
	private String mbCancel = null;

	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (name == null)
		{
			name = "";
			owner = "";
			maxElements = 0;
		}
		if (fullSubmittersList == null)
			fullSubmittersList = new LinkedList();
		if (fullMembersList == null)
			fullMembersList = new LinkedList();
		return RESULT_OK;
	}

	public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
	{
		if (mbCancel != null)
			return cancel();

		int result = super.process(appContext, errors, loginedPrincipal);
		if (result != RESULT_OK)
			return result;

		if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	private int cancel()
	{
		clear();
		return RESULT_DONE;
	}

	protected void clear()
	{
		super.clear();
		mbSave = mbCancel = null;
	}

	private int save()
	{
		DistributionListAdmin admin = appContext.getSmsc().getDistributionListAdmin();
		try
		{
			admin.addDistributionList(new DistributionList(name, maxElements));
			MaskList newSubmitters = new MaskList(fullSubmittersList);
			for (Iterator i = newSubmitters.iterator(); i.hasNext();)
			{
				Mask mask = (Mask) i.next();
				if (mask.getQuestionsCount() > 0)
					return error(SMSCErrors.error.dl.wildcardsNotAllowedInAddress);
				try
				{
					admin.addPrincipal(new Principal(mask.getNormalizedMask(), 0, 0));
				}
				catch (PrincipalAlreadyExistsException e)
				{
					// do nothing
				}
				admin.grantPosting(name, mask.getNormalizedMask());
			}
			MaskList newMembers = new MaskList(fullMembersList);
			for (Iterator i = newMembers.iterator(); i.hasNext();)
			{
				Mask mask = (Mask) i.next();
				if (mask.getQuestionsCount() > 0)
					return error(SMSCErrors.error.dl.wildcardsNotAllowedInAddress);
				admin.addMember(name, mask.getNormalizedMask());
			}
			clear();
			return RESULT_DONE;
		}
		catch (ListAlreadyExistsException e)
		{
			logger.error("Couldn't add distribution list: list \"" + name + "\" already exists");
			return error(SMSCErrors.error.dl.couldntaddAlreadyexists, name);
		}
		catch (AdminException e)
		{
			logger.error("Couldn't add distribution list", e);
			return error(SMSCErrors.error.dl.couldntadd, e);
		}
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
