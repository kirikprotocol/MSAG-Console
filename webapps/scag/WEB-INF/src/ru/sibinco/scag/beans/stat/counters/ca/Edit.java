package ru.sibinco.scag.beans.stat.counters.ca;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.stat.counters.ConfigParam;
import ru.sibinco.scag.backend.stat.counters.Limit;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.backend.stat.counters.CATable;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.*;

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
    private Principal userPrincipal = null;
    private CATable ca_table = new CATable();

    private ConfigParam configParams[] = new ConfigParam[0];
    private Limit limits[]= new Limit[0];

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
        logger.debug("|---------------------------------------------------------------------------") ;
        logger.debug("Counters.process(), save="+getMbSave()+" add="+isAdd()+", id="+getId()); // TODO: remove it
        
        requestParams = request.getParameterMap();
        super.process(request, response);

        userPrincipal = request.getUserPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        // if (isAdd())
    }

    protected void load(String loadId) throws SCAGJspException {
        logger.debug("Loading ca_table, id=" + loadId);
        ca_table = appContext.getCountersManager().getCATables().get(loadId);

        if (ca_table == null) {
            throw new SCAGJspException(Constants.errors.stat.CATABLE_NOT_FOUND, loadId);
        } else{
            limits = getLimitsAsArray(ca_table.getLimits());
            configParams = getConfigParams(ca_table.getParams());
        }
    }

    private ConfigParam[] getConfigParams(Collection<ConfigParam> configParams){
        return (ConfigParam[]) configParams.toArray(new ConfigParam[configParams.size()]);
    }

    public Limit[] getLimitsAsArray(List<Limit> limits){
        Limit[] array = new Limit[limits.size()];
        int counter = 0;
        for (Limit limit : limits) {
            array[counter] = limit;
            counter++;
        }
        return array;
    }

    protected void save() throws SCAGJspException {
        logger.debug("Adding new ca_table, id=" + getId());
        appContext.getCountersManager().addCATable(ca_table);
        // TODO: handle exceptions (if can't add)
        throw new DoneException();
    }

///////////////////////////////////////////////////////////////////////////////
//Methods for working with tables limits.
    public Limit[] getLimits(){
        return this.limits;
    }

    public void setLimits(Limit values[]){
        logger.debug("setLimits: ");
        this.limits = values;
    }

    public void setLimit(int index, Limit value){
        logger.debug("setLimit: "+ value);
        this.limits[index] = value;
    }

    public Limit fggetLimit(int index){
        return this.limits[index];
    }

///////////////////////////////////////////////////////////////////////////////
// Methods for working with tables parameters.
    public ConfigParam[] getParameters(){
        return this.configParams;
    }

    public void setParameters(ConfigParam values[]){
        logger.debug("setParameters: ");
        this.configParams = values;
    }

    public void setParameter(int index, ConfigParam value){
        logger.debug("setParameter: "+ value);
        this.configParams[index] = value;
    }

    public ConfigParam getParameter(int index){
        return this.configParams[index];
    }
}
