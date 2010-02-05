package ru.sibinco.scag.beans.stat.counters.ca;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import java.util.Collection;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 05.02.2010
 * Time: 12:34:25
 */
public class Index extends TabledBeanImpl
{
    protected Collection getDataSource() {
        return appContext.getCountersManager().getCATables().values();
    }

    protected String getDefaultSort() {
        return "id";
    }

    protected void delete() throws SCAGJspException
    {
        // TODO: send command(s) to MSAG
        if (appContext.getCountersManager().getCATables().keySet().removeAll(checkedSet)) {
            try {
                appContext.getCountersManager().apply();
            } catch (Throwable e) {
                logger.debug("Failed to delete c/a table(s)", e);
                try {
                    throw new SCAGJspException(Constants.errors.stat.APPLY_CA_TABLES, e);
                } catch (SCAGJspException e1) {
                    logger.debug("Failed to delete c/a table(s)", e1);
                }
            }
        }
    }
}
