package ru.sibinco.scag.beans.stat.counters;

import ru.sibinco.scag.beans.TabledEditBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.backend.stat.counters.CounterType;
import ru.sibinco.scag.backend.stat.counters.Counter;

import java.util.Collection;
import java.util.Collections;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 02.02.2010
 * Time: 16:19:13
 */
public class Edit extends EditBean
{
    private Counter counter;

    public String getId() {
        return counter.getId();
    }

    protected void load(String loadId) throws SCAGJspException
    {
        counter = appContext.getCountersManager().getCounters().get(loadId);
    }

    protected void save() throws SCAGJspException {
    }

    protected void delete() throws SCAGJspException {
    }

    protected Collection getDataSource() {
        return null;
    }

    public String[] getTypeIds() {
        //return CounterType.values();
        return new String[] {"A", "B", "C"};
    }
    public String[] getCaIds() {
        return new String[] {"a", "b", "c"};
    }
}
