/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SmscBean;

import java.util.*;

public class RoutesAdd extends SmscBean
{
	protected String mbSave = null;
	protected String mbCancel = null;

	protected String routeId = null;
	protected boolean permissible = false;
	protected boolean billing = false;
	protected boolean archiving = false;
	protected String[] checkedSources = null;
	protected String[] srcMasks = null;
	protected String[] checkedDestinations = null;
	protected String[] dstMasks = null;
	protected String dst_mask_sme_ = null;

	protected Set checkedSourcesSet = null;
	protected Set checkedDestinationsSet = null;
	protected Map selectedSmes = new HashMap();
	protected Map selectedMaskSmes = new HashMap();


	protected int init(List errors)
	{
		int result = super.init(errors);
		if (result != RESULT_OK)
			return result;

		if (routeId == null)
			routeId = "";

		if (checkedSources == null)
			checkedSources = new String[0];
		if (checkedDestinations == null)
			checkedDestinations = new String[0];
		if (srcMasks == null)
			srcMasks = new String[0];
		if (dstMasks == null)
			dstMasks = new String[0];

		checkedSources = trimStrings(checkedSources);
		checkedDestinations = trimStrings(checkedDestinations);
		srcMasks = trimStrings(srcMasks);
		dstMasks = trimStrings(dstMasks);

		checkedSourcesSet = new HashSet(Arrays.asList(checkedSources));
		checkedDestinationsSet = new HashSet(Arrays.asList(checkedDestinations));

		for (Iterator i = smsc.getSubjects().iterator(); i.hasNext();)
		{
			Subject subj = (Subject) i.next();
			selectedSmes.put(subj.getName(), subj.getDefaultSme().getId());
		}

		return result;
	}

	public int process(SMSCAppContext appContext, List errors, Map requestParameters)
	{
		int result = super.process(appContext, errors);
		if (result != RESULT_OK)
			return result;

		final String subjprefix = "dst_sme_";
		final String maskprefix = "dst_mask_sme_";
		for (Iterator i = requestParameters.keySet().iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			if (paramName.startsWith(subjprefix))
			{
				final String[] strings = (String[]) requestParameters.get(paramName);
				if (strings.length > 0)
				{
					final String dstName = paramName.substring(subjprefix.length());
					selectedSmes.put(dstName, strings[0]);
				}
			}
			else if (paramName.startsWith(maskprefix))
			{
				final String[] strings = (String[]) requestParameters.get(paramName);
				if (strings.length > 0)
				{
					final String dstName = paramName.substring(maskprefix.length());
					selectedMaskSmes.put(dstName, strings[0]);
				}
			}
		}
		for (int i = 0; i < dstMasks.length; i++)
		{
			String mask = dstMasks[i];
			if (!selectedMaskSmes.containsKey(mask))
				selectedMaskSmes.put(mask, dst_mask_sme_);
		}

		if (mbCancel != null)
			return RESULT_DONE;
		else if (mbSave != null)
			return save();

		return RESULT_OK;
	}

	protected int save()
	{
		if (routeId == null || routeId.length() <= 0)
			return error(SMSCErrors.error.routes.nameNotSpecified);
		if (smsc.getRoutes().contains(routeId))
			return error(SMSCErrors.error.routes.alreadyExists, routeId);

		try
		{
			SourceList sources = new SourceList();
			for (int i = 0; i < checkedSources.length; i++)
			{
				String source = checkedSources[i];
				sources.add(new Source(smsc.getSubjects().get(source)));
			}
			for (int i = 0; i < srcMasks.length; i++)
			{
				String mask = srcMasks[i];
				sources.add(new Source(new Mask(mask)));
			}

			DestinationList destinations = new DestinationList();
			for (int i = 0; i < checkedDestinations.length; i++)
			{
				String destination = checkedDestinations[i];
				Subject subj = smsc.getSubjects().get(destination);
				SME sme = smsc.getSmes().get((String) selectedSmes.get(destination));
				destinations.add(new Destination(subj, sme));
			}
			for (int i = 0; i < dstMasks.length; i++)
			{
				String mask = dstMasks[i];
				String smeId = (String) selectedMaskSmes.get(mask);
				if (smeId == null)
					smeId = dst_mask_sme_;
				SME sme = smsc.getSmes().get(smeId);
				destinations.add(new Destination(new Mask(mask), sme));
			}

			smsc.getRoutes().put(new Route(routeId, permissible, billing, archiving, sources, destinations));
			appContext.getStatuses().setRoutesChanged(true);
			return RESULT_DONE;
		}
		catch (Throwable e)
		{
			return error(SMSCErrors.error.routes.cantAdd, routeId, e);
		}
	}

	public boolean isSrcChecked(String srcName)
	{
		return checkedSourcesSet.contains(srcName);
	}

	public boolean isDstChecked(String dstName)
	{
		return checkedDestinationsSet.contains(dstName);
	}

	public Collection getAllSubjects()
	{
		return smsc.getSubjects().getNames();
	}

	public Collection getAllSmes()
	{
		return smsc.getSmes().getNames();
	}

	public boolean isSmeSelected(String dstName, String smeId)
	{
		return smeId.equals(selectedSmes.get(dstName));
	}

	public boolean isMaskSmeSelected(String dstMask, String smeId)
	{
		return smeId.equals(selectedMaskSmes.get(dstMask));
	}

	/*************************** properties *********************************/

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

	public String getRouteId()
	{
		return routeId;
	}

	public void setRouteId(String routeId)
	{
		this.routeId = routeId;
	}

	public boolean isPermissible()
	{
		return permissible;
	}

	public void setPermissible(boolean permissible)
	{
		this.permissible = permissible;
	}

	public boolean isBilling()
	{
		return billing;
	}

	public void setBilling(boolean billing)
	{
		this.billing = billing;
	}

	public boolean isArchiving()
	{
		return archiving;
	}

	public void setArchiving(boolean archiving)
	{
		this.archiving = archiving;
	}

	public String[] getCheckedSources()
	{
		return checkedSources;
	}

	public void setCheckedSources(String[] checkedSources)
	{
		this.checkedSources = checkedSources;
	}

	public String[] getSrcMasks()
	{
		return srcMasks;
	}

	public void setSrcMasks(String[] srcMasks)
	{
		this.srcMasks = srcMasks;
	}

	public String[] getCheckedDestinations()
	{
		return checkedDestinations;
	}

	public void setCheckedDestinations(String[] checkedDestinations)
	{
		this.checkedDestinations = checkedDestinations;
	}

	public String[] getDstMasks()
	{
		return dstMasks;
	}

	public void setDstMasks(String[] dstMasks)
	{
		this.dstMasks = dstMasks;
	}

	public String getDst_mask_sme_()
	{
		return dst_mask_sme_;
	}

	public void setDst_mask_sme_(String dst_mask_sme_)
	{
		this.dst_mask_sme_ = dst_mask_sme_;
	}
}
