package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.Collection;

public class Index extends TabledBeanImpl {

    protected Collection getDataSource() {
        return appContext.getSmppManager().getCenters().values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getSmppManager().deleteCenters(getUserName(), checkedSet, appContext);
    }

}
