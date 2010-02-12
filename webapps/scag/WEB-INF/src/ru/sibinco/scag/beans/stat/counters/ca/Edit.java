package ru.sibinco.scag.beans.stat.counters.ca;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.backend.stat.counters.CATable;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;
import java.util.HashMap;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 05.02.2010
 * Time: 16:38:15
 */
public class Edit  extends EditBean
{
    private CATable ca_table = new CATable();

    protected Map requestParams = new HashMap();

    public String getId() {
        return ca_table.getId();
    }
    public void setId(String id) {
        ca_table.setId(id);
    }

    public int getLimitsMin() {
        return ca_table.getLimitsMin();
    }
    public void setLimitsMin(int limitsMin) {
        ca_table.setLimitsMin(limitsMin);
    }

    public int getLimitsMax() {
        return ca_table.getLimitsMax();
    }
    public void setLimitsMax(int limitsMax) {
        ca_table.setLimitsMax(limitsMax);
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException
    {
        requestParams = request.getParameterMap();
        super.process(request, response);

        // if (isAdd())
    }

    protected void load(String loadId) throws SCAGJspException {
        logger.debug("Loading ca_table, id=" + loadId);
        ca_table = appContext.getCountersManager().getCATables().get(loadId);
    }

    protected void save() throws SCAGJspException {
        logger.debug("Adding new ca_table, id=" + getId());
        appContext.getCountersManager().addCATable(ca_table);
        // TODO: handle exceptions (if can't add)
        throw new DoneException();
    }
}
