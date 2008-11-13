package ru.sibinco.scag.beans.gw.config;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Statuses;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;
import java.io.IOException;
import java.security.Principal;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends EditBean {

    private Map params = new HashMap();
    private Map requestParams = null;
    private String mbStop;
    private String mbStart;
    private boolean configChanged = false;
    private boolean stopped = false;
    private boolean started = false;
    private static final String COLLAPSING_TREE_PARAM_PREFIX = "collapsing_tree_param.";
//    private boolean isSaved = false;

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        logger.debug( "config/Index.process() start" );
        requestParams = request.getParameterMap();
//        listParameters(requestParams); //debug code
        super.process(request, response);

        if (null != mbStart)
            start();
        else if (null != mbStop)
            stop();
    }

    private void listParameters( Map params ) {
        logger.debug( "\n---------------------------------------" );
        logger.debug( "config/Index.listParameters() params.size()=" + params.size() );
        for( Iterator i = params.entrySet().iterator(); i.hasNext();  ){
            logger.debug( "\tIndex.listParameter().entry='" + i.next() + "'" );
        }
        logger.debug( "\n---------------------------------------" );
    }

    public String getId() {
        return null;
    }

    protected void load(String loadId) throws SCAGJspException {
        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            if (entry.getKey() instanceof String) {
                String key = (String) entry.getKey();
                if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {         // after save() such params exists
                    String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
                    StringBuffer value = new StringBuffer();
                    for (int j = 0; j < ((String[]) entry.getValue()).length; j++) {
                        String valueElem = ((String[]) entry.getValue())[j];
                        value.append(valueElem.trim());
                    }
                    params.put(name, value.toString());
                }
            }
        }
        if (params.size() == 0) {
            final Config gwConfig = appContext.getGwConfig();
            for (Iterator i = gwConfig.getParameterNames().iterator(); i.hasNext();) {
                String name = (String) i.next();
                Object value = gwConfig.getParameter(name);
                if (value instanceof String)
                    params.put(name, value);
                else if (value instanceof Boolean) {
                    Boolean valueBoolean = (Boolean) value;
                    params.put(name, String.valueOf(valueBoolean.booleanValue()));
                } else if (value instanceof Long) {
                    Long aLong = (Long) value;
                    params.put(name, String.valueOf(aLong.longValue()));
                } else if (value instanceof Integer) {
                    Integer integer = (Integer) value;
                    params.put(name, String.valueOf(integer.longValue()));
                }
            }
        }
    }

//    protected void load_new(String loadId) throws SCAGJspException {
//        // load params from URL
////        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
////            Map.Entry entry = (Map.Entry) i.next();
////            if (entry.getKey() instanceof String) {
////                String key = (String) entry.getKey();
////                if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {
////                    String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
////                    StringBuffer value = new StringBuffer();
////                    for (int j = 0; j < ((String[]) entry.getValue()).length; j++) {
////                        String valueElem = ((String[]) entry.getValue())[j];
////                        value.append(valueElem.trim());
////                    }
////                    params.put(name, value.toString());
////                }
////            }
////        }
//
//        logger.debug( "config/Index.load() start, params.size()=" + params.size() );
//        //if no params in request
////        if (params.size() == 0) {
//            final Config gwConfig = appContext.getGwConfig();
//            for (Iterator i = gwConfig.getParameterNames().iterator(); i.hasNext();) {
//                String name = (String) i.next();
//                Object value = gwConfig.getParameter(name);
//                logger.debug( "config.Index.load() name='" + name + "' value='" + value + "'" );
//                if (value instanceof String)
//                    params.put(name, value);
//                else if (value instanceof Boolean) {
//                    Boolean valueBoolean = (Boolean) value;
//                    params.put(name, String.valueOf(valueBoolean.booleanValue()));
//                } else if (value instanceof Long) {
//                    Long aLong = (Long) value;
//                    params.put(name, String.valueOf(aLong.longValue()));
//                } else if (value instanceof Integer) {
//                    Integer integer = (Integer) value;
//                    params.put(name, String.valueOf(integer.longValue()));
//                }
//            }
////        }
//        logger.debug( "config/Index.load() L100, params.size()=" + params.size() );
//        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
//            Map.Entry entry = (Map.Entry) i.next();
//            if (entry.getKey() instanceof String) {
//                String key = (String) entry.getKey();
//                if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {
//                    String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
//                    StringBuffer value = new StringBuffer();
//                    String[] values = (String[]) entry.getValue();
//                    for (int j = 0; j < 1; j++) {
//                        String valueElem = values[j];
//                        logger.debug( "config/Index.load() requestParams value" + j + "='" + value + "'" );
//                        logger.debug( "config/Index.load() requestParams valueElem=" + valueElem + " param=" + params.get( name ) );
//                        if( !isSaved && !valueElem.equals(params.get(name)) ){
//                            logger.debug( "config/Index.load() DIFF valueElem=" + valueElem + " param=" + params.get(name) );
//                            value.append(valueElem.trim());
//                            break;
//                        }
//                    }
//                    logger.debug( "config/Index.load() put name" + "='" + name + "value='" + value + "'" );
//                    params.put(name, value.toString());
//                }
//            }
//        }
//        logger.debug( "config/Index.load() END params.size()=" + params.size() + "\n");
//        listParameters( params );
//        isSaved = false;
//    }

//    private String getValue(String name) {
//        Object value =params.get( name );
//
//        if (value instanceof String)
//            params.put(name, value);
//        else if (value instanceof Boolean) {
//            Boolean valueBoolean = (Boolean) value;
//            params.put(name, String.valueOf(valueBoolean.booleanValue()));
//        } else if (value instanceof Long) {
//            Long aLong = (Long) value;
//            params.put(name, String.valueOf(aLong.longValue()));
//        } else if (value instanceof Integer) {
//            Integer integer = (Integer) value;
//            params.put(name, String.valueOf(integer.longValue()));
//        }
//
//        String result = null;
//        return result;
//    }

    protected void save() throws SCAGJspException {
        logger.debug( "config/Index.save() start" );
        listParameters( params );
        final Config gwConfig = appContext.getGwConfig();
        Config oldConfig = null;
        try {
          oldConfig = (Config)gwConfig.clone();
        } catch (CloneNotSupportedException c) {}
        final Statuses statuses = appContext.getStatuses();
        for (Iterator i = params.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            String key = (String) entry.getKey();
            Object value = entry.getValue();
            final Object parameter = gwConfig.getParameter(key);
            logger.debug( "config/Index.save() key='" + entry.getKey() + "'");
//            logger.debug( "config/Index.save() S key='" + key + "'");
            if (parameter != null) {
                if (parameter instanceof String){
                    logger.debug( "config/Index.save() param S value='" + (String) value + "'");
                    gwConfig.setString(key, (String)value);
                }
                else if (parameter instanceof Integer || parameter instanceof Long){
                    try {
                        logger.debug( "config/Index.save() param I value='" + Integer.parseInt( ( (String)value).trim() ) + "'");
                        gwConfig.setInt( key, Integer.parseInt( ((String)value).trim() ) );
                     } catch (NumberFormatException e) {
                        logger.debug( "config/Index.save() INVALID_INTEGER");
                        throw new SCAGJspException(Constants.errors.config.INVALID_INTEGER, (String) entry.getValue());
                    }
                } else if (parameter instanceof Boolean){
                    logger.debug( "config/Index.save() param  B value='" + ((String)value).trim() + "'");
                    gwConfig.setBool( key, Boolean.valueOf((String)value).booleanValue() );
                } else {
                    logger.debug( "config/Index.save() param O value='" + (String)value + "'");
                    gwConfig.setString(key, ((String)value).trim());
                }
            } else {
                try {
                    logger.debug( "config/Index.save() NEW param I value='" + (String)value + "'");
                    gwConfig.setInt( key, Integer.parseInt( ((String)value).trim() ) );
                } catch (NumberFormatException e) {
                    if( ((String)value).equalsIgnoreCase("true") || ((String)value).equalsIgnoreCase("false") ){
                        logger.debug( "config/Index.save() NEW param B value='" + (String)value + "'");
                        gwConfig.setBool( key, Boolean.valueOf((String)value).booleanValue() );
                    } else {
                        logger.debug( "config/Index.save() NEW param S value='" + (String)value + "'");
                        gwConfig.setString(key, ((String) value).trim());
                    }
                }
            }
            statuses.setConfigChanged(true);
            this.configChanged = true;
        }

        //appContext.getGwConfig().save();
        appContext.getConfigManager().applyConfig(appContext, oldConfig);
        appContext.getStatuses().setConfigChanged(false);
        started = true;
        stopped = true;
        StatMessage message = new StatMessage(getUserName(appContext), "Configuration", "Changed configuration.");
        StatusManager.getInstance().addStatMessages(message);
        throw new DoneException();
    }


    private void stop() throws SCAGJspException {
        try {
            appContext.getScagDaemon().shutdownService(appContext.getScag().getId());
            stopped = false;
        } catch (SibincoException e) {
            logger.error("Could not stop Scag", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_STOP_GATEWAY, e);
        }
    }

    private void start() throws SCAGJspException {
        try {
            appContext.getScagDaemon().startService(appContext.getScag().getId());
            started = false;
        } catch (SibincoException e) {
            logger.error("Could not start Scag", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_START_GATEWAY, e);
        }
    }

    private String getUserName(SCAGAppContext appContext) throws SCAGJspException {
            Principal userPrincipal = loginedPrincipal;

            if (userPrincipal == null)
                throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
            User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
            if (user == null)
                throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
            return user.getName();
        }



    public Map getParams() {
        return params;
    }

    public void setParams(Map params) {
        this.params = params;
    }

    public String getMbStop() {
        return mbStop;
    }

    public void setMbStop(String mbStop) {
        this.mbStop = mbStop;
    }

    public String getMbStart() {
        return mbStart;
    }

    public void setMbStart(String mbStart) {
        this.mbStart = mbStart;
    }

    public boolean isConfigChanged() {
        return configChanged;
    }

    public void setConfigChanged(boolean configChanged) {
        this.configChanged = configChanged;
    }

    public boolean isStopped() {
        return stopped;
    }

    public void setStopped(boolean stopped) {
        this.stopped = stopped;
    }

    public boolean isStarted() {
        return started;
    }

    public void setStarted(boolean started) {
        this.started = started;
    }

    public String getColTreeParamPrefix() {
        return COLLAPSING_TREE_PARAM_PREFIX;
    }

//    public void setColTreeParamPrefix(String str){
//        logger.debug( "config/Index attempt to save COLLAPSING_TREE_PARAM_PREFIX" );;
//    }
}
