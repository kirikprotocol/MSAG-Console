package ru.sibinco.scag.beans.routing.subjects;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;

import java.util.Collection;
import java.security.Principal;


/**
 * Created by igork Date: 19.04.2004 Time: 17:39:13
 */
public class Index extends TabledBeanImpl implements TabledBean {
    protected Collection getDataSource() {
        return appContext.getScagRoutingManager().getSubjects().values();
    }

    protected void delete() {
        appContext.getScagRoutingManager().getSubjects().keySet().removeAll(checkedSet);
        appContext.getScagRoutingManager().setRoutesChanged(true);
        StatMessage message = null;
        try {
            message = new StatMessage(getUserName(appContext), "Subject", "Deleted subject(s): " + checkedSet.toString() + ".");
        } catch (SCAGJspException e) {
            logger.debug("Failed to obtain user", e);
        }
        appContext.getScagRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);
    }

    private String getUserName(SCAGAppContext appContext) throws SCAGJspException {
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        return user.getName();
    }
}
