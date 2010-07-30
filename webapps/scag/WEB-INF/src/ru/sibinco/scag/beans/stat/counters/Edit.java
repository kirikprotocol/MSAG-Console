package ru.sibinco.scag.beans.stat.counters;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.stat.counters.*;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.DoneException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.*;

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
    private Principal userPrincipal = null;
    private Counter counter = new Counter();

    private ConfigParam configParams[] = new ConfigParam[0];

    private String[] caIds;

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

    private String getParameterType(String value){
        //logger.debug("Parameter type: " + value);
        String type;
        if (value.compareTo("true") == 0 || value.compareTo("false") == 0){           
            type = "bool";
        } else{
            try{
                Integer.parseInt(value);
                type = "int";
            } catch (NumberFormatException e){
                type = "string";
            }
        }
        //logger.debug("Type: "+type);
        return type;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException
    {
        logger.debug("|---------------------------------------------------------------------------") ;
        logger.debug("Counters.process(), save="+getMbSave()+" add="+isAdd()+", id="+getId()); // TODO: remove it

        // Read counter's parameters.
        if (getMbSave() != null){
            Enumeration e = request.getParameterNames();
            String key, name, value, type;
            while (e.hasMoreElements()) {
                key = (String) e.nextElement();
                //logger.debug("Paremeter key: "+key);
                if (key.startsWith("parameter")&&(key.endsWith("name"))) {
                    name = request.getParameter(key);
                    //logger.debug("Parameter name: "+ name);
                    value = request.getParameter("parameter." + key.substring(10,key.length()-5)+".value");
                    //logger.debug("Parameter value:"+ value);
                    type = getParameterType(value);
                    //logger.debug(name+"-->"+value);
                    counter.setParam(new ConfigParam(name, type, value));
                }
            }
            configParams = getConfigParams(counter.getParams());
        }

        super.process(request, response);
        userPrincipal = request.getUserPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");

        if (isAdd()){
            final Set<String> cas = appContext.getCountersManager().getCATables().keySet();
            caIds = cas.toArray(new String[cas.size()]);
        }
    }

    private ConfigParam[] getConfigParams(Collection<ConfigParam> configParams){
        return (ConfigParam[]) configParams.toArray(new ConfigParam[configParams.size()]); 
    }

    protected void load(String loadId) throws SCAGJspException {
        final Set<String> cas = appContext.getCountersManager().getCATables().keySet();
        caIds = cas.toArray(new String[cas.size()]);

        logger.debug("Loading counter, id=" + loadId);
        counter = appContext.getCountersManager().getCounters().get(loadId);        
        if (counter == null) {
            throw new SCAGJspException(Constants.errors.stat.COUNTER_NOT_FOUND, loadId);
        }

        configParams = getConfigParams(counter.getParams());
    }

    protected void save() throws SCAGJspException {        
        CountersManager countersManager = appContext.getCountersManager();
        String id = getId();
        Counter oldCounter = null;
        //Counter counter;
        HashMap<String, Counter> counters = countersManager.getCounters();
        if (isAdd()){
            if (!countersManager.isUniqueCounterName(id)){
                logger.error( "counters.Edit:save():counter - name not unique" );
                throw new SCAGJspException( Constants.errors.stat.CAN_NOT_SAVE_COUNTER_NOT_UNIQUE_NAME, id );
            }
        } else {
            logger.debug("Update counter, id=" + getId());
            oldCounter = counters.get(id);
        }
        //counter = new  Counter(id, getType(), getCATableId());

        //ConfigParam[] cp = getParameters();
        //for(int i=0; i<cp.length; i++) counter.setParam( cp[i]);

        counters.remove(getEditId());
        counters.put(id, counter);
        countersManager.createUpdateCounter(getLoginedPrincipal().getName(), isAdd(), counter, appContext, oldCounter);

        // TODO: handle exceptions (if can't add)
        throw new DoneException();
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
        return caIds; 
    }

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
