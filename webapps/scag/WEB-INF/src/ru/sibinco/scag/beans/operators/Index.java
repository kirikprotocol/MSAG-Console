/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.operators;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.lib.backend.users.User;

import java.util.ArrayList;
import java.util.Collection;
import java.security.Principal;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 02.03.2006
 * Time: 13:52:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl {

    protected Collection getDataSource() {
        return appContext.getOperatorManager().getOperators().values();
    }

    protected void delete() throws SCAGJspException {
        final ArrayList toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String operatorIdStr = checked[i];
            final Long operatorId = Long.decode(operatorIdStr);
            toRemove.add(operatorId);
        }
        appContext.getOperatorManager().delete(getUser(appContext).getLogin(), toRemove);
        appContext.getOperatorManager().reloadOperators(appContext.getScag());
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
