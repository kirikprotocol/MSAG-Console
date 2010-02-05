package ru.sibinco.scag.beans.stat.counters;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.backend.stat.counters.Counter;
import ru.sibinco.scag.backend.stat.counters.CounterType;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;
import static java.lang.String.valueOf;

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
    private Counter counter = new Counter();

    protected Map requestParams = new HashMap();
    protected String[] countersIds = new String[0];

    public String getId() {
        return counter.getId();
    }
    public void setId(String id) {
        counter.setId(id);
    }
    public String getType() {
        return counter.getTypeString();
    }
    public void setType(String type) {
        counter.setType(type);
    }

    public String getCATableId() {
        return counter.getCATableId();
    }
    public void setCATableId(String CATableId) {
        counter.setCATableId(CATableId);
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException
    {
        requestParams = request.getParameterMap();
        super.process(request, response);

        /*if (isAdd()) {
            counter = new Counter();
        }*/

        final Collection<Counter> counters = appContext.getCountersManager().getCounters().values();
        final List<String> ids = new ArrayList<String>(counters.size());
        for (Counter counter : counters) ids.add(valueOf(counter.getId()));
        countersIds = ids.toArray(new String[ids.size()]);
    }

    protected void load(String loadId) throws SCAGJspException {
        logger.debug("Loading counter, id=" + loadId);
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
        CounterType[] types = CounterType.values();
        final List<String> ids = new ArrayList<String>(types.length);
        for (CounterType type : types) ids.add(Integer.toString(type.getValue()));
        return ids.toArray(new String[ids.size()]);
    }
    public String[] getTypeTitles() {
        CounterType[] types = CounterType.values();
        final List<String> ids = new ArrayList<String>(types.length);
        for (CounterType type : types) ids.add(type.getName());
        return ids.toArray(new String[ids.size()]);
    }
    public String[] getCaIds() {
        Set<String> cas = appContext.getCountersManager().getCATables().keySet();
        return cas.toArray(new String[cas.size()]);
    }
    
}
