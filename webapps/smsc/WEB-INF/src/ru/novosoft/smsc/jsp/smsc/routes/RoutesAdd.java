/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.routes;

import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.*;

public class RoutesAdd extends RouteBody
{
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

		for (Iterator i = routeSubjectManager.getSubjects().iterator(); i.hasNext();)
		{
			Subject subj = (Subject) i.next();
			selectedSmes.put(subj.getName(), subj.getDefaultSme().getId());
		}

		return result;
	}

	public int process(SMSCAppContext appContext, List errors, Map requestParameters, java.security.Principal loginedPrincipal)
	{
		int result = super.process(appContext, errors, loginedPrincipal);
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
		if (routeSubjectManager.getRoutes().contains(routeId))
			return error(SMSCErrors.error.routes.alreadyExists, routeId);
		if (priority < 0 || priority > MAX_PRIORITY)
			return error(SMSCErrors.error.routes.invalidPriority, String.valueOf(priority));

		try
		{
			SourceList sources = new SourceList();
			for (int i = 0; i < checkedSources.length; i++)
			{
				String source = checkedSources[i];
				sources.add(new Source(routeSubjectManager.getSubjects().get(source)));
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
				Subject subj = routeSubjectManager.getSubjects().get(destination);
				SME sme = smeManager.getSmes().get((String) selectedSmes.get(destination));
				destinations.add(new Destination(subj, sme));
			}
			for (int i = 0; i < dstMasks.length; i++)
			{
				String mask = dstMasks[i];
				String smeId = (String) selectedMaskSmes.get(mask);
				if (smeId == null)
					smeId = dst_mask_sme_;
				SME sme = smeManager.getSmes().get(smeId);
				destinations.add(new Destination(new Mask(mask), sme));
			}

			if (sources.isEmpty())
				return error(SMSCErrors.error.routes.sourcesIsEmpty);
			if (destinations.isEmpty())
				return error(SMSCErrors.error.routes.destinationsIsEmpty);

			routeSubjectManager.getRoutes().put(new Route(routeId, priority, permissible, billing, archiving, suppressDeliveryReports, active, serviceId, sources, destinations));
			appContext.getStatuses().setRoutesChanged(true);
			return RESULT_DONE;
		}
		catch (Throwable e)
		{
			return error(SMSCErrors.error.routes.cantAdd, routeId, e);
		}
	}
}
