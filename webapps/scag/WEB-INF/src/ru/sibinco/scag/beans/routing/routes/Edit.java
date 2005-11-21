package ru.sibinco.scag.beans.routing.routes;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.routing.Destination;
import ru.sibinco.scag.backend.routing.Route;
import ru.sibinco.scag.backend.routing.Source;
import ru.sibinco.scag.backend.routing.Subject;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.Category;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.security.Principal;


/**
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends EditBean {
    private String name;
    private String[] srcMasks = new String[0];
    private String[] srcSubjs = new String[0];
    private String[] dstMasks = new String[0];
    private String[] dstSubjs = new String[0];
    private boolean enabled;
    private boolean archived;
    private boolean active;
    private String srcSmeId;
    private String notes;

    private String category;
    private long categoryId;

    private long ruleId;
    private String rule;

    private String provider;
    private long providerId;

    private Set srcMasksSet = new HashSet();
    private Set srcSubjsSet = new HashSet();
    private Set dstMasksSet = new HashSet();
    private Set dstSubjectsSet = new HashSet();

    private String new_dstMask;
    private String new_dst_mask_sme_;
    private Map destinations;
    private static final String DST_SME_PREFIX = "dst_sme_";
    private static final String DST_MASK_PREFIX = "dst_mask_sme_";


    public String getId() {
        return name;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {

        appContext = getAppContext();
        if(appContext == null){
            appContext = (SCAGAppContext) request.getAttribute("appContext");
        }
        destinations = new HashMap();
        for (Iterator i = request.getParameterMap().entrySet().iterator(); i.hasNext();) {
            final Map.Entry entry = (Map.Entry) i.next();
            final String s = (String) entry.getKey();
            if (s.startsWith(DST_SME_PREFIX)) {
                final String subjName = s.substring(DST_SME_PREFIX.length());
                final String[] smeNameStrings = (String[]) entry.getValue();
                final StringBuffer smeName = new StringBuffer();
                for (int j = 0; j < smeNameStrings.length; j++) {
                    final String smeNameString = smeNameStrings[j];
                    if (null != smeNameString)
                        smeName.append(smeNameString.trim());
                }
                final Subject subj = (Subject) appContext.getScagRoutingManager().getSubjects().get(subjName);
                if (null == subj)
                    throw new SCAGJspException(Constants.errors.routing.routes.DEST_SUBJ_NOT_FOUND, subjName);

                final String smeId = smeName.toString();

                final Svc sme = (Svc) appContext.getSmppManager().getSvcs().get(smeId);
                if (null == sme)
                    throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);

                try {
                    final Destination destination = new Destination(subj, sme);
                    destinations.put(destination.getName(), destination);
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

                    final Svc sme = (Svc) appContext.getSmppManager().getSvcs().get(smeId);
                    if (null == sme)
                        throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);

                    try {
                        final Destination destination = new Destination(mask, sme);
                        destinations.put(destination.getName(), destination);
                    } catch (SibincoException e) {
                        logger.debug("Could not create destination", e);
                        throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
                    }
                }
            }
        }
        if (null != new_dstMask && null != new_dst_mask_sme_ && 0 < new_dstMask.length() && 0 < new_dst_mask_sme_.length()) {
            final Mask mask;
            try {
                mask = new Mask(new_dstMask);
            } catch (SibincoException e) {
                logger.debug("Could not create destination mask", e);
                throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, new_dstMask, e);
            }
            final Svc sme = (Svc) appContext.getSmppManager().getSvcs().get(new_dst_mask_sme_);
            if (null == sme)
                throw new SCAGJspException(Constants.errors.routing.routes.SME_NOT_FOUND, new_dst_mask_sme_);
            try {
                final Destination destination = new Destination(mask, sme);
                destinations.put(destination.getName(), destination);
                if (0 < appContext.getSmppManager().getSvcs().size())
                    new_dst_mask_sme_ = (String) appContext.getSmppManager().getSvcs().keySet().iterator().next();
                new_dstMask = "";
            } catch (SibincoException e) {
                logger.debug("Could not create destination", e);
                throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
            }
        }
        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {
           if (null == new_dst_mask_sme_ && 0 < appContext.getSmppManager().getSvcs().size())
            new_dst_mask_sme_ = (String) appContext.getSmppManager().getSvcs().keySet().iterator().next();

        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(loadId);
        if (null != route) {
            name = route.getName();

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
            srcSmeId = route.getSrcSmeId();
            notes = route.getNotes();

            if (null != route.getProvider()) {
                provider = route.getProvider().getName();
                providerId = route.getProvider().getId();
            }
            if (null != route.getRule()) {
                rule = route.getRule().getName();
                ruleId = route.getRule().getId().longValue();
            }

            if (null != route.getCategory()) {
                category = route.getCategory().getName();
                categoryId = route.getCategory().getId();
            }
        }
    }

    protected void save() throws SCAGJspException {

        final Map routes = appContext.getScagRoutingManager().getRoutes();
        try {
            final Map sources = createSources();
            final Provider providerObj = (Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));
            final Rule ruleObj = (Rule) appContext.getRuleManager().getRules().get(new Long(ruleId));
            final Category categoryObj = (Category) appContext.getCategoryManager().getCategories().get(new Long(categoryId));

            if (isAdd()) {
                if (routes.containsKey(name))
                    throw new SCAGJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, name);
                routes.put(name,
                        new Route(name, sources, destinations, archived, enabled, active, srcSmeId,
                                providerObj, categoryObj, ruleObj, notes));
            } else {
                if (!getEditId().equals(name) && routes.containsKey(name))
                    throw new SCAGJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
                routes.remove(getEditId());
                routes.put(name,
                        new Route(name, sources, destinations, archived, enabled, active, srcSmeId,
                                providerObj, categoryObj, ruleObj, notes));
            }
        } catch (SibincoException e) {
            logger.error("Could not create new subject", e);
            throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
        }
        appContext.getScagRoutingManager().setRoutesChanged(true);
        appContext.getScagRoutingManager().setChangedByUser(getUser(appContext).getName());
        appContext.getScagRoutingManager().setRoutesSaved(true);

        throw new DoneException();
    }

    private Map createSources() throws SibincoException {
        final Map result = new HashMap();
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
        return (String[]) new SortedList(appContext.getSmppManager().getSvcs().keySet()).toArray(new String[0]);
    }

    public String[] getProviderIds() {
        final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
        final List result = new ArrayList(providers.size());
        for (Iterator i = providers.keySet().iterator(); i.hasNext();) {
            result.add(String.valueOf(((Long) i.next()).longValue()));
        }
        return (String[]) result.toArray(new String[0]);
    }

    public String[] getProviders() {
        final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
        final List result = new ArrayList(providers.size());
        for (Iterator i = providers.values().iterator(); i.hasNext();) {
            result.add(((Provider) i.next()).getName());
        }
        return (String[]) result.toArray(new String[0]);
    }

    public String[] getRuleIds() {
        final Map rules = new TreeMap(appContext.getRuleManager().getRules());
        final List result = new ArrayList(rules.size());
        for (Iterator i = rules.keySet().iterator(); i.hasNext();) {
            result.add(String.valueOf(((Long) i.next()).longValue()));
        }
        return (String[]) result.toArray(new String[result.size()]);
    }

    public String[] getRules() {
        final Map rules = new TreeMap(appContext.getRuleManager().getRules());
        final List result = new ArrayList(rules.size());
        for (Iterator i = rules.values().iterator(); i.hasNext();) {
            result.add(((Rule) i.next()).getName());
        }
        return (String[]) result.toArray(new String[result.size()]);
    }

    public String[] getCategoryIds() {
        final Map categories = new TreeMap(appContext.getCategoryManager().getCategories());
        final List result = new ArrayList(categories.size());
        for (Iterator i = categories.keySet().iterator(); i.hasNext();) {
            result.add(String.valueOf(((Long) i.next()).longValue()));
        }
        return (String[]) result.toArray(new String[result.size()]);
    }

    public String[] getCategories() {
        final Map rules = new TreeMap(appContext.getCategoryManager().getCategories());
        final List result = new ArrayList(rules.size());
        for (Iterator i = rules.values().iterator(); i.hasNext();) {
            result.add(((Category) i.next()).getName());
        }
        return (String[]) result.toArray(new String[result.size()]);
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
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

    public String getProvider() {
        return provider;
    }

    public void setProvider(final String provider) {
        this.provider = provider;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(final long providerId) {
        this.providerId = providerId;
    }

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public boolean isArchived() {
        return archived;
    }

    public void setArchived(boolean archived) {
        this.archived = archived;
    }

    public long getRuleId() {
        return ruleId;
    }

    public void setRuleId(long ruleId) {
        this.ruleId = ruleId;
    }

    public String getRule() {
        return rule;
    }

    public void setRule(String rule) {
        this.rule = rule;
    }

    public long getCategoryId() {
        return categoryId;
    }

    public void setCategoryId(long categoryId) {
        this.categoryId = categoryId;
    }

    public List getAllSmes() {
        return new SortedList(appContext.getSmppManager().getSvcs().keySet());
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
            result.put(subject.getName(), subject.getDefaultSme().getId());
        }
        return result;
    }

    public Map getDstSubjPairs() {
        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
        if (null != route) {
            final Map result = new TreeMap();
            for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
                final Destination destination = (Destination) i.next();
                if (destination.isSubject())
                    result.put(destination.getName(), destination.getSvc().getId());
            }
            return result;
        }
        return null;
    }

    public Map getDstMaskPairs() {
        final Route route = (Route) appContext.getScagRoutingManager().getRoutes().get(getEditId());
        if (null != route) {
            final Map result = new TreeMap();
            for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
                final Destination destination = (Destination) i.next();
                if (!destination.isSubject())
                    result.put(destination.getName(), destination.getSvc().getId());
            }
            return result;
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

}
