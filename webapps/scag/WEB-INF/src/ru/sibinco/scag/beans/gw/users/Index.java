package ru.sibinco.scag.beans.gw.users;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import java.util.Collection;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends TabledBeanImpl implements TabledBean {

    protected Collection getDataSource() {
        return appContext.getUserManager().getUsers().values();
    }

    protected String getDefaultSort() {
        return "login";
    }

    protected void delete() {
        if (appContext.getUserManager().getUsers().keySet().removeAll(checkedSet)) {
            try {
                appContext.getUserManager().apply();
                appContext.getStatuses().setUsersChanged(false);
            } catch (Throwable e) {
                logger.debug("Couldn't apply users", e);
                try {
                    throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_USERS, e);
                } catch (SCAGJspException e1) {
                    logger.debug("Couldn't apply users", e1);
                }
            }
        }
    }
}
