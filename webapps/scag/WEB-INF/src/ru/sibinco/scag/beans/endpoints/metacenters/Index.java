package ru.sibinco.scag.beans.endpoints.metacenters;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.Collection;

public class Index extends TabledBeanImpl {


    protected Collection getDataSource() {
        return appContext.getSmppManager().getMetaCenters().values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getSmppManager().deleteMetaCenters(getUserName(), checkedSet, appContext);
    }

}
