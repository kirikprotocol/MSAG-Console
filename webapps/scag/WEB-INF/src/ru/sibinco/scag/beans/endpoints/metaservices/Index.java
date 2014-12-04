package ru.sibinco.scag.beans.endpoints.metaservices;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.Collection;

public class Index extends TabledBeanImpl {

    protected Collection getDataSource() {
        return appContext.getSmppManager().getMetaServices().values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getSmppManager().deleteMetaServices(getUserName(), checkedSet, appContext);
    }

}
