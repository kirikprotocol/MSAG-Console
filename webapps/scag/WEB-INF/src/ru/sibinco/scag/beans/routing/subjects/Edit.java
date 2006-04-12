package ru.sibinco.scag.beans.routing.subjects;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.route.MaskList;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.routing.Subject;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.security.Principal;


/**
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends EditBean {
    private String name;
    private String defaultSme;
    private String[] masks = new String[0];
    private String description;

    public String getId() {
        return name;
    }

    protected void load(final String loadId) throws SCAGJspException {
        final Subject subject = (Subject) appContext.getScagRoutingManager().getSubjects().get(loadId);
        if (null != subject) {
            name = subject.getName();
            defaultSme = null != subject.getSvc() ? subject.getSvc().getId() : null;
            if(defaultSme == null){
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
    }

    protected void save() throws SCAGJspException {
        String messagetxt = "";
        masks = Functions.trimStrings(masks);
        final Map subjects = appContext.getScagRoutingManager().getSubjects();
        final Svc defSvc = (Svc) appContext.getSmppManager().getSvcs().get(defaultSme);
        final Center defCenter = (Center) appContext.getSmppManager().getCenters().get(defaultSme);
        if (defSvc == null && defCenter == null)
            throw new SCAGJspException(Constants.errors.routing.subjects.DEFAULT_SME_NOT_FOUND, defaultSme);

        if (isAdd()) {
            if (subjects.containsKey(name))
                throw new SCAGJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
            try {
                subjects.put(name, defSvc == null ? new Subject(name, defCenter, masks, description) : new Subject(name, defSvc, masks, description));
                messagetxt = "Added new subject: '" + name + "'.";
            } catch (SibincoException e) {
                logger.debug("Could not create new subject", e);
                throw new SCAGJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
            }
        } else {
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
        }
        appContext.getScagRoutingManager().setRoutesChanged(true);
        appContext.getScagRoutingManager().setChangedByUser(getUser(appContext).getName());
        StatMessage message = new StatMessage(getUser(appContext).getName(), "Subject", messagetxt);
        appContext.getScagRoutingManager().addStatMessages(message);
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

    public String getDescription() {
        return description;
    }

    public void setDescription(final String description) {
        this.description = description;
    }
}
