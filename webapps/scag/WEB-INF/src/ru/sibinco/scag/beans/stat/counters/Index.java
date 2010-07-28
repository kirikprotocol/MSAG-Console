package ru.sibinco.scag.beans.stat.counters;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import java.util.Collection;
import java.util.HashMap;

public class Index extends TabledBeanImpl
{    

    protected Collection getDataSource() {
        return appContext.getCountersManager().getCounters().values();
    }

    protected String getDefaultSort() {
        return "id";
    }

    protected void delete() throws SCAGJspException
    {
        if (appContext.getCountersManager().getCounters().keySet().removeAll(checkedSet)) {
            try {
                appContext.getCountersManager().apply();
            } catch (Throwable e) {
                logger.debug("Failed to delete counters", e);
                try {
                    throw new SCAGJspException(Constants.errors.stat.APPLY_COUNTERS, e);
                } catch (SCAGJspException e1) {
                    logger.debug("Failed to delete counters", e1);
                }
            }
        }
    }

}
