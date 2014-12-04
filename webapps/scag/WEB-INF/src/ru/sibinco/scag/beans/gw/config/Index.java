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

public class Index extends EditBean {

    private Map params = new HashMap();
    private Map requestParams = null;
    private String mbStop;
    private String mbStart;
    private boolean configChanged = false;
    private boolean stopped = false;
    private boolean started = false;

    protected boolean diskio;

    public boolean isDiskio() {
        logger.debug( "config/Index.isDiskio()=" + diskio );
        return diskio;
    }

    public void setDiskio(boolean diskio) {
        logger.debug( "config/Index.setDiskio()=" + diskio );
        this.diskio = diskio;
    }

    protected boolean enabled;

    public boolean isEnabled() {
        logger.debug( "config/Index.isEnabled()=" + enabled );
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        logger.debug( "config/Index.setEnabled()=" + enabled );
        this.enabled = enabled;
    }


    private String test = "myvalue";

    public String getTest() {
        return test;
    }

    public void setTest(String test) {
        this.test = test;
    }

    private static final String COLLAPSING_TREE_PARAM_PREFIX = "collapsing_tree_param.";
//    private boolean isSaved = false;

//    boolean boo = false;

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {

        initDefaults();

        logger.debug( "config/Index.process() start" );
//        setDiskio( true );

//        logger.debug( "config/Index.process() start diskio='" + isDiskio() + "'" );

        requestParams = request.getParameterMap();
        logger.debug( "config/Index.process() listParameters(requestParams)" );
        listParameters(requestParams); //debug code
        super.process(request, response);

        if (null != mbStart)
            start();
        else if (null != mbStop)
            stop();
    }

    static final Map DEFAULT_VALUE_I = new HashMap(10);

    private void initDefaults() {
        initDefaultsI();
//        initDefaultsS();
    }

    private void initDefaultsI() {
        DEFAULT_VALUE_I.put( "smpp.core.bindWaitTimeout", new Integer(10) );
        DEFAULT_VALUE_I.put( "smpp.core.connectionsPerIp", new Integer(100) );
        DEFAULT_VALUE_I.put( "smpp.core.ipBlockingTime", new Integer(60) );
        DEFAULT_VALUE_I.put( "smpp.core.maxRWMultiplexersCount", new Integer(100) );
        DEFAULT_VALUE_I.put( "smpp.core.socketsPerMultiplexer", new Integer(16) );
        ;
    }

    private Integer getDefaultI( String key ) {
        return (Integer)DEFAULT_VALUE_I.get( key );
    }

    void printAttribs( HttpServletRequest request ){
        Enumeration en = request.getAttributeNames();
        while( en.hasMoreElements() ){
            logger.debug( "config/Index.printAttribs() attributeName='" + (String)en.nextElement() + "'" );
        }
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
        logger.debug( "Index.load() start \nlistParameters( requestParams )" );
        listParameters( requestParams );
        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {

            Map.Entry entry = (Map.Entry) i.next();
            String keyExt = (String)entry.getKey();
            logger.debug( "Index.load() keyExt='" + keyExt + "'" );

            if (entry.getKey() instanceof String) {
                String key = (String) entry.getKey();
                logger.debug( "Index.load() from requestPram key='" + key + "'" );

                if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {         // after save() such params exists
                    String name = key.substring(COLLAPSING_TREE_PARAM_PREFIX.length());
                    StringBuffer value = new StringBuffer();
                    for (int j = 0; j < ((String[]) entry.getValue()).length; j++) {
                        String valueElem = ((String[]) entry.getValue())[j];
                        logger.debug( "Index.load() " + j + ". valueElem='" + valueElem + "'" );
                        value.append(valueElem.trim());
                    }

                    logger.debug( "Index.load() from requestPram name='" + name + "' value='" + value );
                    params.put(name, value.toString());
                }
            }
        }
        logger.debug( "Index.load() params.size()='" + params.size() + "'" );
        if( params.size() == 0 ){
            final Config gwConfig = appContext.getGwConfig();
            for (Iterator i = gwConfig.getParameterNames().iterator(); i.hasNext();) {
                String name = (String) i.next();
                logger.debug( "Index.load() name='" + name + "'" );
                Object value = gwConfig.getParameter(name);

                setBools( name, value );

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
        logger.debug( "config/Index.load() end listParameters(params)" );
        listParameters( params );
    }

    protected void save() throws SCAGJspException {
        logger.debug( "config/Index.save() start" );
        //print params in log
        logger.debug( "config/Index.save() listParameters(params)" );
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
            logger.debug( "config/Index.save() key='" + key + "'--- value='" + value + "'" );

            if (parameter != null) {
                logger.debug( "config/Index.save() (parameter != null)");
                if (parameter instanceof String){
                    logger.debug( "config/Index.save() param S value='" + (String) value + "'");
                    gwConfig.setString(key, (String)value);
                } else if (parameter instanceof Integer || parameter instanceof Long){
                    try {
                        int iValue = Integer.parseInt( ((String)value).trim() );
                        logger.debug( "config/Index.save() param I value='" + iValue + "'");
                        gwConfig.setInt( key, iValue );
                    } catch (NumberFormatException e) {
                        logger.debug( "config/Index.save() INVALID_INTEGER, key='" + key + "' value='" + value + "'");
                            throw new SCAGJspException(Constants.errors.config.INVALID_INTEGER, (String) entry.getValue());
                    }
                } else if (parameter instanceof Boolean){
                    logger.debug( "config/Index.save() param  B value='" + Boolean.valueOf((String)value).booleanValue() + "'");
                    gwConfig.setBool( key, Boolean.valueOf((String)value).booleanValue() );
                } else {
                    logger.debug( "config/Index.save() param O value='" + (String)value + "'");
                    gwConfig.setString(key, ((String)value).trim());
                }
            } else {    // new parameter added at page
                try {
                    logger.debug( "config/Index.save() NEW param I value='" + (String)value + "'");
                    gwConfig.setInt( key, Integer.parseInt( ((String)value).trim() ) );
                } catch (NumberFormatException e) {
                    if( getDefaultI( key ) != null ){
                        logger.debug( "config/Index.save() NEW param I set default value='" + getDefaultI( key ) + "'");
                        gwConfig.setInt( key, getDefaultI(key).intValue() );
                    } else if( ((String)value).equalsIgnoreCase("true") || ((String)value).equalsIgnoreCase("false") ){
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
        logger.debug( "config/Index.save() appContext.getStatuses().setConfigChanged(false)" );
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
            if (userName == null)
                throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user name");
            User user = (User) appContext.getUserManager().getUsers().get(userName);
            if (user == null)
                throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userName + "'");
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

    void setBools( String name, Object value ){
        if( name.indexOf( ".diskio" ) != -1 ){
            logger.debug( "Index.load() diskio='" + String.valueOf( ((Boolean)value).booleanValue() ) + "'" );
            if( "true".equalsIgnoreCase( String.valueOf( ((Boolean)value).booleanValue() ) ) ){
                logger.debug( "Index.load() set diskio to TRUE" );
                setDiskio( true );
            } else{
                logger.debug( "Index.load() set diskio to FALSE" );
                setDiskio( false );
            }
        }
        if( name.indexOf( ".enabled" ) != -1 ){
            logger.debug( "Index.load() enabled='" + String.valueOf( ((Boolean)value).booleanValue() ) + "'" );
            if( "true".equalsIgnoreCase( String.valueOf( ((Boolean)value).booleanValue() ) ) ){
                logger.debug( "Index.load() set enabled to TRUE" );
                setEnabled( true );
            } else{
                logger.debug( "Index.load() set enabled to FALSE" );
                setEnabled( false );
            }
        }
    }

//    public void setColTreeParamPrefix(String str){
//        logger.debug( "config/Index attempt to save COLLAPSING_TREE_PARAM_PREFIX" );;
//    }
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

