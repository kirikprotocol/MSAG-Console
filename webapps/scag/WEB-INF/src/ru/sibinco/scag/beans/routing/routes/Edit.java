package ru.sibinco.scag.beans.routing.routes;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.scag.backend.routing.Destination;
import ru.sibinco.scag.backend.routing.Route;
import ru.sibinco.scag.backend.routing.Source;
import ru.sibinco.scag.backend.routing.Subject;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.CancelChildException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.security.Principal;
import java.util.*;


/**
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends EditBean {//TabledEditBeanImpl {

    public static final long ALL_PROVIDERS = -1;
    private String id;
    private String[] srcMasks = new String[0];
    private String[] srcSubjs = new String[0];
    private String[] dstMasks = new String[0];
    private String[] dstSubjs = new String[0];
    private boolean enabled;
    private boolean archived;
    private boolean active;
    private boolean transit;
    private boolean saa;
    private boolean hideSaaText;
    private String[] slicingTypes = new String[]{"NONE","SAR","UDH8", "UDH16"};
    private String[] slicingTypesTitles = new String[]{"NO/OFF","Via SAR Fields", "Via UDH8 encoding", "Via UDH16 encoding"};
    private String slicing;
    private String slicedRespPolicy;
    private String[] slicedRespPolicyTypes = new String[]{"ALL","ANY"};
    private String[] slicedRespPolicyTypesTitles = new String[]{"All","Any"};
    private String srcSmeId;
    private String notes;
    private String path = "";
    private String parentId;
    private String serviceName = null;
    private boolean administrator = false;

    private Set srcMasksSet = new HashSet();
    private Set srcSubjsSet = new HashSet();
    private Set dstMasksSet = new HashSet();
    private Set dstSubjectsSet = new HashSet();

    private String new_dstMask;
    private String new_dst_mask_sme_;
    private Map destinations;
    private Map destSubjs;
    private Map destMasks;
    private static final String DST_SME_PREFIX = "dst_sme_";
    private static final String DST_MASK_PREFIX = "dst_mask_sme_";
    private HttpSession session;
    private boolean emptyDestinations = false;

    public String getId() {
        return id;
    }


    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        logger.debug("routing/routes/Edit:process():start" );
        logger.debug("routing/routes/Edit process() saa='" + saa + "'");
        logger.debug("routing/routes/Edit process() hideSaaText='" + hideSaaText + "'");
        path = request.getContextPath();
        appContext = getAppContext();
        session = request.getSession();
        emptyDestinations = false;
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }
        if (getMbCancel() != null) {
            logger.debug("routing/routes/Edit:process():getMbCancel()");
            storeToSessionParentGetFlag(request, Constants.GSP_TRUE);
            throw new CancelChildException(new StringBuffer().append(path).
                    append("/services/service/edit.jsp?parentId=").append(getParentId()).
                    append("&editId=").append(appContext.getServiceProviderManager().getServiceProviderByServiceId(
                    Long.decode(getParentId())).getId()).append("&editChild=true").toString());
        }
        if (getEditId() != null){
            id = getEditId();
        }
        destinations = new HashMap();
        destSubjs = new HashMap();
        destMasks = new HashMap();
        logger.debug("routing/routes/Edit.java:process():request.getParameterMap().entrySet()=" + request.getParameterMap().entrySet());
        for (Iterator i = request.getParameterMap().entrySet().iterator(); i.hasNext();) {
            final Map.Entry entry = (Map.Entry) i.next();
            final String s = (String) entry.getKey();
            logger.error( "key=" + s + " | value='" + entry.getValue().getClass() + "'" );
            if (s.startsWith(DST_SME_PREFIX)) {
                final String subjName = s.substring(DST_SME_PREFIX.length());
                logger.debug( "startsWith(DST_SME_PREFIX):subjName=" + subjName);
                final String[] smeNameStrings = (String[]) entry.getValue();
                logger.debug( "startsWith(DST_SME_PREFIX):smeNameStrings=" + smeNameStrings);
                final StringBuffer smeName = new StringBuffer();
                for (int j = 0; j < smeNameStrings.length; j++) {
                    final String smeNameString = smeNameStrings[j];
                    logger.debug( "startsWith(DST_SME_PREFIX):" + j + "smeNameString=" + smeNameString);
                    if (null != smeNameString)
                        smeName.append(smeNameString.trim());
                }
                final Subject subj = (Subject) appContext.getScagRoutingManager().getSubjects().get(subjName);
                if (null == subj)
                    throw new SCAGJspException(Constants.errors.routing.routes.DEST_SUBJ_NOT_FOUND, subjName);

                final String smeId = smeName.toString();

                final Svc svc = (Svc) appContext.getSmppManager().getSvcs().get(smeId);
                final Center center = (Center) appContext.getSmppManager().getCenters().get(smeId);
                final MetaEndpoint mService = (MetaEndpoint) appContext.getSmppManager().getMetaServices().get(smeId);
                final MetaEndpoint mCenter = (MetaEndpoint) appContext.getSmppManager().getMetaCenters().get(smeId);
                if( svc == null && center == null && mService == null && mCenter == null){
                    logger.error( "routing/routes/Edit:process():SCAGJspException");
                    throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);
                }


                try {
//                    final Destination destination = svc == null ? new Destination(subj, center) : new Destination(subj, svc);
                    Destination destination;
                    String id = null;
                    if( svc != null ) {destination = new Destination(subj, svc); id = svc.getId();}
                    else if( center != null ){ destination = new Destination(subj, center); id = center.getId();}
                    else if( mService != null ){ destination = new Destination(subj, mService); id = mService.getId();}
                    else{ destination = new Destination(subj, mCenter); id = mCenter.getId();}
                    destinations.put(destination.getName(), destination);
                    destSubjs.put(destination.getName(), id);
                } catch (SibincoException e) {
                    logger.debug("Could not create destination", e);
                    throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
                }
            } else if (s.startsWith(DST_MASK_PREFIX)) {
                final String maskName = s.substring(DST_MASK_PREFIX.length());
                if (null != maskName && 0 < maskName.trim().length()) {
                    final String[] smeNameStrings = (String[]) entry.getValue();
                    final StringBuffer smeName = new StringBuffer();
                    for (int j = 0; j < smeNameStrings.length; j++) {
                        final String smeNameString = smeNameStrings[j];
                        if (null != smeNameString)
                            smeName.append(smeNameString.trim());
                    }
                    final Mask mask;
                    try {
                        mask = new Mask(maskName);
                    } catch (SibincoException e) {
                        logger.debug("Could not create destination mask", e);
                        throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, maskName, e);
                    }

                    final String smeId = smeName.toString();

                    final Svc svc = (Svc) appContext.getSmppManager().getSvcs().get(smeId);
                    final Center center = (Center) appContext.getSmppManager().getCenters().get(smeId);
                    final MetaEndpoint mService = (MetaEndpoint) appContext.getSmppManager().getMetaServices().get(smeId);
                    final MetaEndpoint mCenter = (MetaEndpoint) appContext.getSmppManager().getMetaCenters().get(smeId);
                    if( null == svc && center == null && null == mService && mCenter == null ){
                        logger.error( "2");
                        throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);
                    }

                    try {
//                        final Destination destination = svc == null ? new Destination(mask, center) : new Destination(mask, svc);
                        Destination destination;
                        String id = null;
                        if( svc != null ){ destination = new Destination(mask, svc); id = svc.getId();}
                        else if( center != null ){ destination = new Destination(mask, center); id = center.getId();}
                        else if( mService != null ){ destination = new Destination(mask, mService); id = mService.getId();}
                        else{ destination = new Destination(mask, mCenter); id = mCenter.getId();}
                        destinations.put(destination.getName(), destination);
                        destMasks.put(destination.getName(), id);
                    } catch (SibincoException e) {
                        logger.error("Could not create destination ", e);
                        throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
                    }
                }
            }
        }
        if (null != new_dstMask && null != new_dst_mask_sme_ && 0 < new_dstMask.length() && 0 < new_dst_mask_sme_.length())
        {
            final Mask mask;
            try {
                mask = new Mask(new_dstMask);
            } catch (SibincoException e) {
                logger.debug("Could not create destination mask ", e);
                throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, new_dstMask, e);
            }
            final Svc svc = (Svc) appContext.getSmppManager().getSvcs().get(new_dst_mask_sme_);
            final Center center = (Center) appContext.getSmppManager().getCenters().get(new_dst_mask_sme_);
            final MetaEndpoint mService = (MetaEndpoint) appContext.getSmppManager().getMetaServices().get(new_dst_mask_sme_);
            final MetaEndpoint mCenter = (MetaEndpoint) appContext.getSmppManager().getMetaCenters().get(new_dst_mask_sme_);
            if( null == svc && center == null && null == mService && mCenter == null ){
                logger.error( "3");
                throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, new_dst_mask_sme_);
            }
            try {
//                final Destination destination = svc == null ? new Destination(mask, center) : new Destination(mask, svc);
                Destination destination;
                if( svc != null ) destination = new Destination(mask, svc);
                else if( center != null ) destination = new Destination(mask, center);
                else if( mService != null ) destination = new Destination(mask, mService);
                else destination = new Destination(mask, mCenter);
                destinations.put(destination.getName(), destination);
                if (0 < appContext.getSmppManager().getSvcs().size())
                    new_dst_mask_sme_ = (String) appContext.getSmppManager().getSvcs().keySet().iterator().next();
                new_dstMask = "";
            } catch (SibincoException e) {
                logger.debug("Could not create destination ", e);
                throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
            }
        }
        if (getMbSave() != null) {
            logger.debug("routing/routes/Edit:process():getMbSave()");
            storeToSessionParentGetFlag(request, Constants.GSP_TRUE);
            super.process(request, response);
            logger.debug("routing/routes/Edit:process():getMbSave()1");
            save();
        }
        logger.debug("Edit.java:process():load(id):id=" + id);
        load(id);
        super.process(request, response);

    }

    void storeToSessionParentGetFlag( final HttpServletRequest request, String value ){
        request.getSession().setAttribute(Constants.GET_FROM_SESSION_START_POSITION_SS, value);
        logger.debug("routing/routes/Edit:storeToSessionGetFlag()"  +
                      request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_SS) );
    }

    protected Collection getDataSource() {
        return new LinkedList();
    }

    protected void delete() throws SCAGJspException {

    }

    protected void load(final String loadId) throws SCAGJspException {
        logger.debug("routing/routes/Edit:load():start");
        if (null == new_dst_mask_sme_ && 0 < appContext.getSmppManager().getSvcs().size())
            new_dst_mask_sme_ = (String) appContext.getSmppManager().getSvcs().keySet().iterator().next();

        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
        if (null != route) {
            id = route.getName();

            final List subjList = new ArrayList();
            final List maskList = new ArrayList();
            for (Iterator i = route.getSources().values().iterator(); i.hasNext();) {
                final Source source = (Source) i.next();
                if (source.isSubject()) {
                    subjList.add(source.getName());
                    srcSubjsSet.add(source.getName());
                } else {
                    maskList.add(source.getName());
                    srcMasksSet.add(source.getName());
                }
            }
            srcMasks = (String[]) maskList.toArray(new String[0]);
            srcSubjs = (String[]) subjList.toArray(new String[0]);

            maskList.clear();
            subjList.clear();

            for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
                final Destination destination = (Destination) i.next();
                if (destination.isSubject()) {
                    subjList.add(destination.getName());
                    dstSubjectsSet.add(destination.getName());
                } else {
                    maskList.add(destination.getName());
                    dstMasksSet.add(destination.getName());
                }
            }
            dstMasks = (String[]) maskList.toArray(new String[0]);
            dstSubjs = (String[]) subjList.toArray(new String[0]);

            enabled = route.isEnabled();
            archived = route.isArchived();
            active = route.isActive();
            transit = route.isTransit();
            saa = route.isSaa();
            hideSaaText = route.isHideSaaText();
            slicing = route.getSlicing();
            slicedRespPolicy= route.getSlicedRespPolicy();
            srcSmeId = route.getSrcSmeId();
            notes = route.getNotes();

            if (null != route.getService()) {
                serviceName = route.getService().getName();
            }
        }
        logger.debug("routing/routes/Edit.load() saa='"+ saa + "' end");
        logger.debug("routing/routes/Edit.load() hideSaaText='"+ hideSaaText + "' end");
        if (isAdd()) {
            enabled = true;
            active = true;
            saa = true;
            hideSaaText = true;
        }
    }

    protected void save() throws SCAGJspException {
        logger.debug("routing/routes/Edit:save():start saa='" + saa + "'");
        logger.debug("routing/routes/Edit:save():start hideSaaText='" + hideSaaText + "'");
        final Map routes = appContext.getScagRoutingManager().getRoutes();
        String messagetxt = "";
        try {
            final Map sources = createSources();
            srcMasks = removeEmptyFromArr( srcMasks );
            dstMasks = removeEmptyFromArr( dstMasks );
            if( sources == null || sources.isEmpty() ){
                logger.warn( "routing/routes/Edit:save():empty SOURCE MAP|srcMasks.length is:'" + srcMasks.length + "'" );
                if( destinations == null || destinations.isEmpty() ){
                    logger.warn( "routing/routes/Edit:save():empty DESTINATONS MAP|destinations" );
                    emptyDestinations = true;
                    throw new SCAGJspException( Constants.errors.routing.routes.CAN_NOT_SAVE_ROUTE_SOUR_DEST );
                }
                throw new SCAGJspException(Constants.errors.routing.routes.CAN_NOT_SAVE_ROUTE_SOUR);
            }
            if( destinations == null || destinations.isEmpty() ){
                logger.warn("Edit.java:save():empty DESTINATION MAP: editId=" + getEditId());
                final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
                emptyDestinations = true;
                throw new SCAGJspException(Constants.errors.routing.routes.CAN_NOT_SAVE_ROUTE_DEST);
            }

            final Service serviceObj = appContext.getServiceProviderManager().getServiceById(Long.decode(getParentId()));
            if (isAdd()) {
                if( routes.containsKey(id) ){
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, id);
                }
//                routes.put( id, new Route(id, sources, destinations, isArchived(), isEnabled(), isActive(),
//                                         getSlicing(), getSlicedRespPolicy(), srcSmeId, serviceObj, notes) );
                routes.put( id, new Route(id, sources, destinations, isArchived(), isEnabled(), isActive(),
                                         getSlicing(), getSlicedRespPolicy(), srcSmeId, serviceObj, notes, transit, saa, hideSaaText ) );
                messagetxt = "Added new route: " + id + " ";
            } else {
                if (!getEditId().equals(id) && routes.containsKey(id)){
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, id);
                }
                routes.remove(getEditId());
//                routes.put( id, new Route(id, sources, destinations, archived, enabled, active,
//                                         getSlicing(), getSlicedRespPolicy(), srcSmeId, serviceObj, notes) );
                routes.put( id, new Route(id, sources, destinations, archived, enabled, active,
                                         getSlicing(), getSlicedRespPolicy(), srcSmeId, serviceObj, notes, transit, saa, hideSaaText ) );
                messagetxt = "Changed route: " + id + " ";
            }
        } catch (SibincoException e) {
            logger.error("Could not create new route ", e);
            throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE, e);
        }
        logger.debug("Edit.java:save():middle");
        appContext.getScagRoutingManager().setRoutesChanged(true);
        appContext.getScagRoutingManager().setChangedByUser(getUser(appContext).getName());
        appContext.getScagRoutingManager().setRoutesSaved(true);

        StatMessage message = new StatMessage(getUser(appContext).getName(), "Routes" , messagetxt);
        appContext.getScagRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);

        throw new CancelChildException( new StringBuffer().append(path).
                  append("/services/service/edit.jsp?parentId=").
                  append(getParentId()).append("&editId=").append(
                  appContext.getServiceProviderManager().getServiceProviderByServiceId(
                  Long.decode(getParentId())).getId()).append("&editChild=true").toString() );
    }

    private Map createSources() throws SibincoException {
        final Map result = new HashMap();
        logger.debug("routing/routes/Edit:createSources():start");
        for (int i = 0; i < srcSubjs.length; i++) {
            final String srcSubj = srcSubjs[i];
            final Subject subject = (Subject) appContext.getScagRoutingManager().getSubjects().get(srcSubj);
            if (null != subject) {
                final Source source = new Source(subject);
                result.put(source.getName(), source);
            }
        }
        for (int i = 0; i < srcMasks.length; i++) {
            final String srcMask = srcMasks[i];
            if (null != srcMask && 0 < srcMask.trim().length()) {
                final Mask mask = new Mask(srcMask);
                final Source source = new Source(mask);
                result.put(source.getName(), source);
            }
        }
        return result;
    }

    public String[] getSmeIds() {
        String[] svcs = (String[]) new SortedList(appContext.getSmppManager().getSvcs().keySet()).toArray(new String[0]);
        String[] centers = (String[]) new SortedList(appContext.getSmppManager().getCenters().keySet()).toArray(new String[0]);
        String[] result = new String[svcs.length + centers.length];
        System.arraycopy(svcs, 0, result, 0, svcs.length);
        System.arraycopy(centers, 0, result, svcs.length, centers.length);
        return result;
    }

    public String[] getCategoryIds() {
        final Map categories = new TreeMap(appContext.getCategoryManager().getCategories());
        final List result = new ArrayList(categories.size());
        for (Iterator i = categories.keySet().iterator(); i.hasNext();) {
            result.add(String.valueOf(((Long) i.next()).longValue()));
        }
        return (String[]) result.toArray(new String[result.size()]);
    }


    public void setId(final String id) {
        this.id = id;
    }

    public String[] getSrcMasks() {
        return srcMasks;
    }

    public void setSrcMasks(final String[] srcMasks) {
        this.srcMasks = srcMasks;
    }

    public String[] getSrcSubjs() {
        return srcSubjs;
    }

    public void setSrcSubjs(final String[] srcSubjs) {
        this.srcSubjs = srcSubjs;
    }

    public String[] getDstMasks() {
        return dstMasks;
    }

    public void setDstMasks(final String[] dstMasks) {
        this.dstMasks = dstMasks;
    }

    public String[] getDstSubjs() {
        return dstSubjs;
    }

    public void setDstSubjs(final String[] dstSubjs) {
        this.dstSubjs = dstSubjs;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isActive() {
        return active;
    }

    public void setActive(final boolean active) {
        this.active = active;
    }

    public String getSrcSmeId() {
        return srcSmeId;
    }

    public void setSrcSmeId(final String srcSmeId) {
        this.srcSmeId = srcSmeId;
    }

    public String getNotes() {
        return notes;
    }

    public void setNotes(final String notes) {
        this.notes = notes;
    }

    public boolean isArchived() {
        return archived;
    }

    public void setArchived(boolean archived) {
        this.archived = archived;
    }

    public String[] getSlicingTypes() {
        return slicingTypes;
    }

    public String[] getSlicedRespPolicyTypes() {
        return slicedRespPolicyTypes;
    }

    public String[] getSlicingTypesTitles() {
        return slicingTypesTitles;
    }

    public String[] getSlicedRespPolicyTypesTitles() {
        return slicedRespPolicyTypesTitles;
    }

    public String getSlicing() {
        return slicing;
    }

    public void setSlicing(final String slicing) {
        this.slicing = slicing;
    }

    public String getSlicedRespPolicy() {
        return slicedRespPolicy;
    }

    public void setSlicedRespPolicy(final String slicedRespPolicy) {
        this.slicedRespPolicy = slicedRespPolicy;
    }

    public List getAllSmes() {
        SortedList list = new SortedList();
        list.addAll(appContext.getSmppManager().getServicesByMetaGroupId("").keySet());
        list.addAll(appContext.getSmppManager().getCentersByMetaGroupId("").keySet());
        list.addAll(appContext.getSmppManager().getMetaServices().keySet());
        list.addAll(appContext.getSmppManager().getMetaCenters().keySet());
        return list;
    }

    public List getAllUncheckedSrcSubjects() {
        final SortedList srcSubjs = new SortedList(appContext.getScagRoutingManager().getSubjects().keySet());
        srcSubjs.removeAll(srcSubjsSet);
        return srcSubjs;
    }

    public Map getAllUncheckedDstSubjects() {
        final Map result = new TreeMap();
        for (Iterator i = appContext.getScagRoutingManager().getSubjects().values().iterator(); i.hasNext();) {
            final Subject subject = (Subject) i.next();
            logger.info( "routing/routes/Edit:getAllUncheckedDstSubjects():dstSubjectsSet=" + dstSubjectsSet);
            logger.info( "routing/routes/Edit:getAllUncheckedDstSubjects():getSubject from routingmanager=" + subject.getId());

            if( !dstSubjectsSet.contains( subject.getId()) ){
                result.put( subject.getName(),
                    subject.getSvc() != null ? subject.getSvc().getId():
                    subject.getCenter() != null ? subject.getCenter().getId():
                    subject.getMetaSvc() != null ? subject.getMetaSvc().getId():
                    subject.getMetaCenter().getId() );
            }
        }
        return result;
    }

    public Map getDstSubjPairs() {
        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
        Map result = new TreeMap();
        if (null != route) {
            if( !emptyDestinations ){
                for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
                    final Destination destination = (Destination) i.next();
                    if (destination.isSubject())
//                        result.put(destination.getName(), destination.getSvc() == null ? destination.getCenter().getId() : destination.getSvc().getId());
                      result.put( destination.getName(),
                            destination.getSvc() != null ? destination.getSvc().getId():
                            destination.getCenter() != null ? destination.getCenter().getId():
                            destination.getMetaSvc() != null ? destination.getMetaSvc().getId():
                            destination.getMetaCenter().getId() );

                }
            }
            return result;

        } else if (isAdd() ){
            return destSubjs;
        }

        return null;
    }

    public Map getDstMaskPairs() {
        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
        final Map result = new TreeMap();
        if( route != null) {
//            final Map result = new TreeMap();
            if( !emptyDestinations ){
                for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
                    final Destination destination = (Destination) i.next();
                    if (!destination.isSubject() )
                        result.put(destination.getName(),
//                                destination.getSvc() == null ? destination.getCenter().getId() : destination.getSvc().getId());
                                destination.getSvc() != null ? destination.getSvc().getId():
                                destination.getCenter() != null? destination.getCenter().getId() :
                                destination.getMetaSvc() != null? destination.getMetaSvc().getId():
                                destination.getMetaCenter().getId());
                }
            }
            return result;

        } else if (isAdd() ){
            return destMasks;
        }

        return null;
    }

    public String getNew_dstMask() {
        return new_dstMask;
    }

    public void setNew_dstMask(final String new_dstMask) {
        this.new_dstMask = new_dstMask;
    }

    public String getNew_dst_mask_sme_() {
        return new_dst_mask_sme_;
    }

    public void setNew_dst_mask_sme_(final String new_dst_mask_sme_) {
        this.new_dst_mask_sme_ = new_dst_mask_sme_;
    }

    private User getUser(SCAGAppContext appContext) throws SCAGJspException {
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        return user;
    }

    public boolean isAdministrator() {
        return administrator;
    }

    public String getServiceName() {
        return serviceName;
    }

    public String getParentId() {
        return parentId;
    }

    public void setParentId(String parentId) {
        this.parentId = parentId;
    }

    public HttpSession getSession() {
        return session;
    }

    public String[] removeEmptyFromArr( String[] srcMasks ){
        int index=0;
        int count=0;

        for( int i = 0; i < srcMasks.length; i++){
            if( srcMasks[i].trim().length() != 0 ){
                count++;
            }
        }
        String[] result = new String[count];
        logger.debug("routing/routes/Edit:removeEmptyFromArr():not empty count='" + count + "'");

        for( int i = 0; i < srcMasks.length; i++){
            if( srcMasks[i].trim().length() != 0 ){
                result[index++] = srcMasks[i];
            }
        }
        return result;
    }

    public boolean isTransit() {
        return transit;
    }

    public void setTransit(final boolean transit) {
        this.transit = transit;
    }

    public boolean isSaa() {
        return saa;
    }

    public void setSaa(final boolean saa) {
        this.saa = saa;
    }

    public boolean isHideSaaText(){
        return this.hideSaaText;
    }

    public void setHideSaaText(final boolean hideSaaText){
        this.hideSaaText = hideSaaText;
    }
}
