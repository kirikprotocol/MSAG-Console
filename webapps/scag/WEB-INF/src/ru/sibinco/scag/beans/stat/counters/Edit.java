package ru.sibinco.scag.beans.stat.counters;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.stat.counters.*;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.web.security.AuthFilter;
import ru.sibinco.scag.web.security.UserLoginData;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;

public class Edit extends EditBean{

    private String userName = null;
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
                //logger.debug("Parameter key: "+key);
                if (key.startsWith("parameter")&&(key.endsWith("name"))) {
                    name = request.getParameter(key);
                    //logger.debug("Parameter name: "+ name);
                    value = request.getParameter("parameter." + key.substring(10,key.length()-5)+".value");
                    //logger.debug("Parameter value:"+ value);
                    type = ConfigParam.getParameterType(value);
                    //logger.debug(name+"-->"+value);
                    counter.setParam(new ConfigParam(name, type, value));
                }
            }
            configParams = ConfigParam.getConfigParams(counter.getParams());
        }

        

        super.process(request, response);

        HttpSession session = request.getSession();
        UserLoginData userLoginData = (UserLoginData) session.getAttribute(AuthFilter.USER_LOGIN_DATA);
        userName = userLoginData.getName();

        if (userName == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user name");
        User user = (User) appContext.getUserManager().getUsers().get(userName);
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userName + "'");

        if (isAdd()){
            final Set<String> cas = appContext.getCountersManager().getCATables().keySet();
            caIds = cas.toArray(new String[cas.size()]);
        }
    }



    protected void load(String loadId) throws SCAGJspException {
        final Set<String> cas = appContext.getCountersManager().getCATables().keySet();
        caIds = cas.toArray(new String[cas.size()]);

        logger.debug("Loading counter, id=" + loadId);
        counter = appContext.getCountersManager().getCounters().get(loadId);        
        if (counter == null) {
            throw new SCAGJspException(Constants.errors.stat.COUNTER_NOT_FOUND, loadId);
        }

        configParams = ConfigParam.getConfigParams(counter.getParams());
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

        counters.remove(getEditId());
        counters.put(id, counter);
        countersManager.createUpdateCounter(getUserName(), isAdd(), counter, appContext, oldCounter);
        
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
        this.configParams = values;
    }

    public void setParameter(int index, ConfigParam value){       
        this.configParams[index] = value;
    }

    public ConfigParam getParameter(int index){
        return this.configParams[index];
    }

    
    
}
