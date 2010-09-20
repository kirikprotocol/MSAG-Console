package ru.novosoft.smsc.jsp.smsc.routes;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */

import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.security.Principal;
import java.util.*;


public class RoutesEdit extends RouteBody {

    protected String oldRouteId = null;


    protected int init(final List errors) {
        int result = super.init(errors);
        if (RESULT_OK != result)
            return result;

        for (Iterator i = routeSubjectManager.getSubjects().iterator(); i.hasNext();) {
            final Subject subj = (Subject) i.next();
            selectedSmes.put(subj.getName(), subj.getDefaultSme().getId());
        }

        if (null == routeId)
            routeId = "";
        if (null == srcSmeId)
            srcSmeId = "";

        if (null == oldRouteId) {
            oldRouteId = routeId;
            final Route r = routeSubjectManager.getRoutes().get(routeId);
            if (null == r)
                result = error(SMSCErrors.error.routes.nameNotSpecified);
            else {
                priority = r.getPriority();
                trafficMode = r.getTrafficMode();
                billing = r.getBilling();
                archiving = r.isArchiving();
                serviceId = r.getServiceId();
                srcSmeId = r.getSrcSmeId();
                backupSmeId = r.getBackupSmeId();
                suppressDeliveryReports = r.isSuppressDeliveryReports();
                active = r.isActive();
                checkedSources = (String[]) r.getSources().getSubjectNames().toArray(new String[0]);
                srcMasks = (String[]) r.getSources().getMaskNames().toArray(new String[0]);
                checkedDestinations = (String[]) r.getDestinations().getSubjectNames().toArray(new String[0]);
                dstMasks = (String[]) r.getDestinations().getMaskNames().toArray(new String[0]);
                dst_mask_sme_ = 0 < smeManager.getSmeNames().size() ? (String) smeManager.getSmeNames().iterator().next() : "";
                deliveryMode = r.getDeliveryMode();
                forwardTo = r.getForwardTo();
                hide = r.isHide();
                replayPath = r.getReplayPath();
                notes = r.getNotes();

                for (int i = 0; i < checkedDestinations.length; i++) {
                    final String destination = checkedDestinations[i];
                    selectedSmes.put(destination.trim(), r.getDestinations().get(destination).getSme().getId());
                }
                for (int i = 0; i < dstMasks.length; i++) {
                    final String mask = dstMasks[i];
                    selectedMaskSmes.put(mask, r.getDestinations().get(mask).getSme().getId());
                }
                aclId = r.getAclId();
                allowBlocked = r.isAllowBlocked();
                forceDelivery = r.isForceDelivery();
                providerId = r.getProviderId();
                categoryId = r.getCategoryId();
                transit = r.isTransit();
            }
        }

        if (null == deliveryMode) deliveryMode = "default";
        if (null == forwardTo) forwardTo = "";
        if (null == checkedSources) checkedSources = new String[0];
        if (null == checkedDestinations) checkedDestinations = new String[0];
        if (null == srcMasks) srcMasks = new String[0];
        if (null == dstMasks) dstMasks = new String[0];

        checkedSources = Functions.trimStrings(checkedSources);
        checkedDestinations = Functions.trimStrings(checkedDestinations);
        srcMasks = Functions.trimStrings(srcMasks);
        dstMasks = Functions.trimStrings(dstMasks);

        checkedSourcesSet = new HashSet(Arrays.asList(checkedSources));
        checkedDestinationsSet = new HashSet(Arrays.asList(checkedDestinations));

        return result;
    }

    public int process(final HttpServletRequest request) {
        final int result = super.process(request);
        if (RESULT_OK != result)
            return result;

        final String subjprefix = "dst_sme_";
        final String maskprefix = "dst_mask_sme_";
        final Map requestParameters = request.getParameterMap();
        for (Iterator i = requestParameters.keySet().iterator(); i.hasNext();) {
            final String paramName = (String) i.next();
            if (paramName.startsWith(subjprefix)) {
                final String[] strings = (String[]) requestParameters.get(paramName);
                if (0 < strings.length) {
                    final String dstName = StringEncoderDecoder.decodeHEX(paramName.substring(subjprefix.length()));
                    selectedSmes.put(dstName, strings[0]);
                    System.out.println("TEST: " + dstName + "   " + strings[0] + "  " + paramName.substring(subjprefix.length()));
                }
            } else if (paramName.startsWith(maskprefix)) {
                final String[] strings = (String[]) requestParameters.get(paramName);
                if (0 < strings.length) {
                    final String dstName = StringEncoderDecoder.decodeHEX(paramName.substring(maskprefix.length()));
                    selectedMaskSmes.put(dstName, strings[0]);
                }
            }
        }
        for (int i = 0; i < dstMasks.length; i++) {
            final String mask = dstMasks[i];
            if (!selectedMaskSmes.containsKey(mask))
                selectedMaskSmes.put(mask, dst_mask_sme_);
        }

        if (null != mbCancel)
            return RESULT_DONE;
        else if (null != mbSave)
            return save(request);

        return result;
    }

    protected int save(final HttpServletRequest request) {
        Route oldRoute = null;
        if (null == routeId || 0 >= routeId.length() || null == oldRouteId || 0 >= oldRouteId.length())
            return error(SMSCErrors.error.routes.nameNotSpecified);
        if (0 > priority || Constants.MAX_PRIORITY < priority)
            return error(SMSCErrors.error.routes.invalidPriority, String.valueOf(priority));

        if (!routeId.equals(oldRouteId) && routeSubjectManager.getRoutes().contains(routeId))
            return error(SMSCErrors.error.routes.alreadyExists, routeId);

        try {
            final SourceList sources = new SourceList();
            for (int i = 0; i < checkedSources.length; i++) {
                final String source = checkedSources[i];
                sources.add(new Source(routeSubjectManager.getSubjects().get(source)));
            }
            for (int i = 0; i < srcMasks.length; i++) {
                final String mask = srcMasks[i];
                sources.add(new Source(new Mask(mask)));
            }

            final DestinationList destinations = new DestinationList();
            for (int i = 0; i < checkedDestinations.length; i++) {
                final String destination = checkedDestinations[i];
                final Subject subj = routeSubjectManager.getSubjects().get(destination);
                final SME sme = smeManager.get((String) selectedSmes.get(destination));
                destinations.add(new Destination(subj, sme));
            }
            for (int i = 0; i < dstMasks.length; i++) {
                final String mask = dstMasks[i];
                String smeId = (String) selectedMaskSmes.get(mask);
                if (null == smeId)
                    smeId = dst_mask_sme_;
                final SME sme = smeManager.get(smeId);
                destinations.add(new Destination(new Mask(mask), sme));
            }

            if (sources.isEmpty())
                return error(SMSCErrors.error.routes.sourcesIsEmpty);
            if (destinations.isEmpty())
                return error(SMSCErrors.error.routes.destinationsIsEmpty);
            if ((providerIdStr != null && providerIdStr.length() > 0))
                providerId = Long.parseLong(providerIdStr);
            if ((categoryIdStr != null && categoryIdStr.length() > 0))
                categoryId = Long.parseLong(categoryIdStr);
            oldRoute = routeSubjectManager.getRoutes().remove(oldRouteId);
            Route r = new Route(routeId, priority, trafficMode, billing, transit, archiving, suppressDeliveryReports, active, serviceId, sources, destinations, srcSmeId,
                    deliveryMode, forwardTo, hide, replayPath, notes, forceDelivery, aclId, allowBlocked, providerId, categoryId);
            r.setBackupSmeId(backupSmeId);
            routeSubjectManager.getRoutes().put(r);
            if (oldRouteId.equals(routeId))
                journalAppend(SubjectTypes.TYPE_route, routeId, Actions.ACTION_MODIFY);
            else
                journalAppend(SubjectTypes.TYPE_route, routeId, Actions.ACTION_MODIFY, "old route ID", oldRouteId);
            appContext.getStatuses().setRoutesChanged(true);
            request.getSession().setAttribute("ROUT_ID", routeId);
            return RESULT_DONE;
        }
        catch (Throwable e) {
            if (oldRoute != null) routeSubjectManager.getRoutes().put(oldRoute);
            return error(SMSCErrors.error.routes.cantAdd, routeId, e);
        }
    }

    public String getOldRouteId() {
        return oldRouteId;
    }

    public void setOldRouteId(final String oldRouteId) {
        this.oldRouteId = oldRouteId;
    }

}
