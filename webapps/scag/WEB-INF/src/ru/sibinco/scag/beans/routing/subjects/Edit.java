package ru.sibinco.scag.beans.routing.subjects;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.routing.MaskList;
import ru.sibinco.scag.backend.routing.Subject;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;
import ru.sibinco.scag.backend.routing.http.HttpSubject;
import ru.sibinco.scag.backend.routing.http.HttpSite;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.*;


/**
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends TabledEditBeanImpl {//EditBean {

    private String name;
    private String defaultSme;
    private String[] masks = new String[0];
    private String[] address = new String[0];

    private String description;

    protected long transportId = Transport.SMPP_TRANSPORT_ID;
    private String httpSubjId = null;
    private String httpSiteId = null;
    private int subjectType;
    private Map sites = new HashMap();
    private String mbAddSite;


    public String getId() {
        return name;
    }

    protected Collection getDataSource() {
        return sites.values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getHttpRoutingManager().deleteSite(getId(), getLoginedPrincipal().getName(), checkedSet);
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);

        if (mbDelete != null) {
            delete();
            load(getId());
        }

        if (getId() == null && !isAdd())
            load(getEditId());

        if (getMbAddSite() != null) {
            throw new AddChildException(request.getContextPath() + "/routing/subjects/site", getId());
        }

        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {

        if (transportId == Transport.SMPP_TRANSPORT_ID) {
            final Subject subject = (Subject) appContext.getScagRoutingManager().getSubjects().get(loadId);
            if (null != subject) {
                name = subject.getName();
                defaultSme = null != subject.getSvc() ? subject.getSvc().getId() : null;
                if (defaultSme == null) {
                    defaultSme = null != subject.getCenter() ? subject.getCenter().getId() : null;
                }
                description = subject.getNotes();

                final List maskList = new ArrayList();
                for (Iterator i = subject.getMasks().iterator(); i.hasNext();) {
                    final Mask mask = (Mask) i.next();
                    maskList.add(mask.getMask());
                }
                masks = (String[]) maskList.toArray(new String[0]);
                maskList.clear();

            }
        } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
            if (getSubjectType() == HttpRoutingManager.HTTP_SUBJECT_TYPE) {
                HttpSubject httpSubject = (HttpSubject) appContext.getHttpRoutingManager().getSubjects().get(getHttpSubjId());
                if (null != httpSubject) {
                    name = httpSubject.getName();
                    final List addressList = new ArrayList();
                    for (Iterator i = httpSubject.getMasks().iterator(); i.hasNext();) {
                        final Mask mask = (Mask) i.next();
                        addressList.add(mask.getMask());
                    }
                    address = (String[]) addressList.toArray(new String[addressList.size()]);
                    addressList.clear();
                }
            } else if (getSubjectType() == HttpRoutingManager.HTTP_SITE_TYPE) {

                HttpSite httpSite = (HttpSite) appContext.getHttpRoutingManager().getSites().get(getHttpSiteId());
                if (null != httpSite) {
                    sites = httpSite.getSites();
                    name = httpSite.getName();
                }
            }
        }
    }

    protected void save() throws SCAGJspException {
        String messagetxt = "";
        masks = Functions.trimStrings(masks);
        address = Functions.trimStrings(address);

        final Map subjects = appContext.getScagRoutingManager().getSubjects();
        final Map httpSubjects = appContext.getHttpRoutingManager().getSubjects();
        final Map httpSites = appContext.getHttpRoutingManager().getSites();
        final Svc defSvc = (Svc) appContext.getSmppManager().getSvcs().get(defaultSme);
        final Center defCenter = (Center) appContext.getSmppManager().getCenters().get(defaultSme);
        if (transportId == Transport.SMPP_TRANSPORT_ID) {
            if (defSvc == null && defCenter == null)
                throw new SCAGJspException(Constants.errors.routing.subjects.DEFAULT_SME_NOT_FOUND, defaultSme);
        }

        if (isAdd()) {
            if (transportId == Transport.SMPP_TRANSPORT_ID) {
                if (subjects.containsKey(name))
                    throw new SCAGJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
                try {
                    subjects.put(name, defSvc == null ? new Subject(name, defCenter, masks, description) : new Subject(name, defSvc, masks, description));
                    messagetxt = "Added new subject: '" + name + "'.";
                } catch (SibincoException e) {
                    logger.debug("Could not create new subject", e);
                    throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
                }
            } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
                if (getSubjectType() == HttpRoutingManager.HTTP_SUBJECT_TYPE) {
                    if (httpSubjects.containsKey(name))
                        throw new SCAGJspException(Constants.errors.routing.subjects.HTTP_SUBJECT_ALREADY_EXISTS, name);
                    try {
                        httpSubjects.put(name, new HttpSubject(name, address));
                        messagetxt = "Added new http subject: '" + name + "'.";
                    } catch (SibincoException e) {
                        logger.debug("Could not create new http subject", e);
                        throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE_HTTP_SUBJECT, e);
                    }
                } else if (getSubjectType() == HttpRoutingManager.HTTP_SITE_TYPE) {
                    if (httpSites.containsKey(name))
                        throw new SCAGJspException(Constants.errors.routing.sites.HTTP_SITE_ALREADY_EXISTS, name);//
                    try {
                        httpSites.put(name, new HttpSite(name));
                        messagetxt = "Added new http subject site: '" + name + "'.";
                    } catch (SibincoException e) {
                        logger.debug("Could not create new http subject site", e);
                        throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE_HTTP_SUBJECT_SITE, e);
                    }
                }
            }
        } else {
            if (transportId == Transport.SMPP_TRANSPORT_ID) {
                if (!getEditId().equals(name)) {
                    if (subjects.containsKey(name))
                        throw new SCAGJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
                    subjects.remove(getEditId());
                    try {
                        subjects.put(name, defSvc == null ? new Subject(name, defCenter, masks, description) : new Subject(name, defSvc, masks, description));

                    } catch (SibincoException e) {
                        logger.debug("Could not create new subject", e);
                        throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
                    }
                } else {
                    final Subject subject = (Subject) subjects.get(name);
                    if (defSvc != null) {
                        subject.setSvc(defSvc);
                        subject.setCenter(null);
                    } else {
                        subject.setCenter(defCenter);
                        subject.setSvc(null);
                    }
                    try {
                        subject.setMasks(new MaskList(masks));
                    } catch (SibincoException e) {
                        logger.debug("Could not set masks list for subject", e);
                        throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_SET_MASKS, e);
                    }
                    subject.setNotes(description);
                    messagetxt = "Changed subject: '" + subject.getName() + "'.";
                }
            } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
                if (getSubjectType() == HttpRoutingManager.HTTP_SUBJECT_TYPE) {
                    if (!getId().equals(name)) {
                        if (httpSubjects.containsKey(name))
                            throw new SCAGJspException(Constants.errors.routing.subjects.HTTP_SUBJECT_ALREADY_EXISTS, name);
                        httpSubjects.remove(getId());
                        try {
                            httpSubjects.put(name, new HttpSubject(name, address));
                        } catch (SibincoException e) {
                            logger.debug("Could not create new subject", e);
                            throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
                        }
                    } else {
                        final HttpSubject httpSubject = (HttpSubject) httpSubjects.get(name);
                        try {
                            httpSubject.setMasks(new MaskList(address));
                        } catch (SibincoException e) {
                            logger.debug("Could not set masks list http for subject", e);
                            throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_SET_MASKS, e);
                        }
                        messagetxt = "Changed http subject: '" + httpSubject.getName() + "'.";
                    }
                } else if (getSubjectType() == HttpRoutingManager.HTTP_SITE_TYPE) {

                }
            }
        }
        StatMessage message = new StatMessage(getUser(appContext).getName(), "Subject", messagetxt);
        if (transportId == Transport.SMPP_TRANSPORT_ID) {
            appContext.getScagRoutingManager().setRoutesChanged(true);
            appContext.getScagRoutingManager().setChangedByUser(getUser(appContext).getName());
            appContext.getScagRoutingManager().addStatMessages(message);
        } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
            appContext.getHttpRoutingManager().setRoutesChanged(true);
            appContext.getHttpRoutingManager().addStatMessages(message);
        }
        StatusManager.getInstance().addStatMessages(message);
        throw new DoneException();
    }

    public String getSmeIds() {
        final StringBuffer result = new StringBuffer();
        SortedList smes = new SortedList();
        smes.addAll(appContext.getSmppManager().getSvcs().keySet());
        smes.addAll(appContext.getSmppManager().getCenters().keySet());
        for (Iterator i = smes.iterator(); i.hasNext();) {
            result.append((String) i.next());
            if (i.hasNext())
                result.append(",");
        }
        return result.toString();
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

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public String getDefaultSme() {
        return defaultSme;
    }

    public void setDefaultSme(final String defaultSme) {
        this.defaultSme = defaultSme;
    }

    public String[] getMasks() {
        return masks;
    }

    public void setMasks(final String[] masks) {
        this.masks = masks;
    }

    public String[] getAddress() {
        return address;
    }

    public void setAddress(final String[] address) {
        this.address = address;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(final String description) {
        this.description = description;
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }

    public String getHttpSubjId() {
        return httpSubjId;
    }

    public void setHttpSubjId(String httpSubjId) {
        this.httpSubjId = httpSubjId;
    }

    public String getHttpSiteId() {
        return httpSiteId;
    }

    public void setHttpSiteId(String httpSiteId) {
        this.httpSiteId = httpSiteId;
    }

    public int getSubjectType() {
        return subjectType;
    }

    public void setSubjectType(int subjectType) {
        this.subjectType = subjectType;
    }

    public String getMbAddSite() {
        return mbAddSite;
    }

    public void setMbAddSite(String mbAddSite) {
        this.mbAddSite = mbAddSite;
    }

}
