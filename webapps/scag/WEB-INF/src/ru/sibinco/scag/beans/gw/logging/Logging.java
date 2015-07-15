package ru.sibinco.scag.beans.gw.logging;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;

public class Logging extends EditBean {
    public static final String catParamNamePrefix = "category_";
    private LoggerCategoryInfo rootCategory;
    private String[] priorities = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "NOTSET"};
    private String category;
    private String mbSavePermanent = null;
    private String mbRead = null;
    private boolean running = true;
    private Map<String, LoggerCategoryInfo> fullNameToCatInfo = new TreeMap<String, LoggerCategoryInfo>();

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        logger.debug( "Logging.process() start" );
        super.process(request, response);

        if (getMbApply() != null) {
            logger.debug( "Logging.process() getMbApply()" );
            apply(request.getParameterMap());
            init();
        } else if (getMbSavePermanent() != null) {
            logger.debug( "Logging.process() getMbSavePermanent()" );
            savePermanent(request.getParameterMap());
            init();
        } else if (getMbRead() != null) {
            logger.debug( "Logging.process() getMbRead()" );
            readFLF( true );
        } else {
            logger.debug( "Logging.process() init()" );
            init();
        }

    }

    public class LoggerCategoryInfo implements Comparable {
        private String name;
        private String fullName;
        private String priority;
        private Map<String, LoggerCategoryInfo> children;

        public LoggerCategoryInfo(String name, String fullName, String priority) {
            this.name = name;
            this.fullName = fullName;
            this.priority = priority;
            this.children = new TreeMap<String, LoggerCategoryInfo>();
        }

        public void addChild(String childFullName, String childPriority) {
            if (logger.isDebugEnabled()) logger.debug("Try to add child with full name '"+childFullName+"' and " +
                "priority '"+childPriority+"' to logger category info with name '"+name+"', full name '"+fullName+"', "+
                "priority '"+priority+"'.");

            if (isRoot()) {
                if (logger.isDebugEnabled()) logger.debug("Detected that target logger category info is root category.");
                final int dotPos = childFullName.indexOf('.');
                if (dotPos > 0) {
                    final String childName = childFullName.substring(0, dotPos);
                    if (logger.isDebugEnabled()) logger.debug("Got child name '"+childName+"' from child full name '"+childFullName+"'.");
                    LoggerCategoryInfo child = getOrCreateChild(childName, childName, "NOTSET");
                    child.addChild(childFullName, childPriority);
                } else {
                    LoggerCategoryInfo child = new LoggerCategoryInfo(childFullName, childFullName, childPriority);
                    children.put(childFullName, child);
                    if (logger.isDebugEnabled()) logger.debug("put childFullName --> child: "+childFullName+" --> "+child);
                }
            } else {
                if (childFullName.length() > fullName.length() && childFullName.startsWith(fullName) && childFullName.charAt(fullName.length()) == '.')
                {
                    final int beginIndex = fullName.length() + 1;
                    final int endIndex = childFullName.indexOf('.', beginIndex);
                    final String childName = endIndex > 0 ? childFullName.substring(beginIndex, endIndex) : childFullName.substring(beginIndex);
                    if (logger.isDebugEnabled()) logger.debug("Got child name '"+childName+"' from child full name '"+childFullName+"'.");
                    if (endIndex > 0) {
                        LoggerCategoryInfo child = getOrCreateChild(childName, childFullName.substring(0, endIndex), "NONSET");
                        child.addChild(childFullName, childPriority);
                    } else {
                        LoggerCategoryInfo child = getOrCreateChild(childName, childFullName, childPriority);
                        child.priority = childPriority;
                        if (logger.isDebugEnabled()) logger.debug("Set priority '"+childPriority+" for child "+child+".");
                    }
                } else {
                    logger.debug("Incorrect LoggerCategoryInfo.addChild algorithm");
                }
            }
        }

        private LoggerCategoryInfo getOrCreateChild(String childName, String childFullName, String childPriority) {
            LoggerCategoryInfo child = children.get(childName);
            if (child == null) {
                child = new LoggerCategoryInfo(childName, childFullName, childPriority);
                if (logger.isDebugEnabled()) logger.debug("Create child "+child);
                children.put(childName, child);
                if (logger.isDebugEnabled()) logger.debug("put childName --> child: "+childName+" --> "+child);
            }
            return child;
        }

        public boolean isRoot() {
            return fullName == null || fullName.length() == 0;
        }

        public boolean isHasChilds() {
            return !children.isEmpty();
        }

        public int compareTo(Object o) {
            if (o instanceof LoggerCategoryInfo) {
                LoggerCategoryInfo info = (LoggerCategoryInfo) o;
                return name.compareTo(info.name);
            } else
                return 0;
        }

        public String getName() {
            return name;
        }

        public String getFullName() {
            return fullName;
        }

        public String getPriority() {
            return priority;
        }

        public Map<String,LoggerCategoryInfo> getChilds() {
            return children;
        }

        public String toString() {
            return "\nLoggerCategoryInfo{" +
                "name='"+ name +"', "+
                "fullName='" + fullName + "', "+
                "priority='" + priority + "',"+
                "children: "+ children.values() + "}";
        }
    }

    protected void init() throws SCAGJspException{
        logger.warn( "Logging:init() start" );
        Map logCategories = new HashMap();
        try {
            logCategories = appContext.getScag().getLogCategories();
        } catch (SibincoException e) {
            logger.warn( "Logging.init() SibincoException" );
            if( e instanceof StatusDisconnectedException ){
                setRunning(false);
                logger.warn( "Logging.init() StatusDisconnectedException Disconnected" );
            }
            try{
                logger.warn( "Logging.init() SibincoException readFromLogFile()" );
                logCategories = appContext.getLoggingManager().readFromLogFile();
            }catch (SibincoException e1) {
                logger.error( "Logging.init() SibincoException. readFromLogFile() SibincoException: can not read loggers from file!!!" );
                rootCategory = new LoggerCategoryInfo("", "", "NOTSET");
            }
            throw new SCAGJspException(Constants.errors.logging.COULDNT_GET_LOGCATS, e);
        } finally{
            logger.warn( "Logging.init() SibincoException finally" );
            parseMap( logCategories );
            getLoggerCategoryInfo(rootCategory, fullNameToCatInfo);
        }
    }

    boolean isConnected(){
        return (appContext.getScag().getStatus() == Proxy.STATUS_CONNECTED);
    }

    void readFLF( boolean full ) throws SCAGJspException {
        logger.warn( "Logging.readFLF() start" );
        Map<String, String> logCategoriesFF;
        Map<String, String> logCategories;
        try {
            logger.warn( "Logging.readFLF() readFromLogFile()" );
            logCategoriesFF = appContext.getLoggingManager().readFromLogFile();
            if( !isConnected() ) setRunning( false );
            if( full && isRunning() ){
                logger.warn( "Logging:readFLF() getLogCategories()" );
                logCategories = appContext.getScag().getLogCategories();
                for (String key : logCategories.keySet()) {
                    String valFF = logCategoriesFF.get(key);
                    if (valFF != null) {
                        logCategories.put(key, valFF);
                    } else {
                        logCategories.put(key, "NOTSET");
                    }
                }
            } else {
                logCategories = logCategoriesFF;
            }

            parseMap( logCategories );
        } catch (SibincoException e) {
            logger.warn( "Logging:readFLF() SibincoException", e);
        }
        if( !isRunning() ){
            throw new SCAGJspException( Constants.errors.logging.COULDNT_GET_LOGCATS,
                    new StatusDisconnectedException( appContext.getScag().getHost(), appContext.getScag().getPort()) );
        }
    }

    public void parseMap( Map logCategories ){
        String rootPriority = (String) logCategories.remove("");
        if (rootPriority == null) {
            rootPriority = "NOTSET";
            if (logger.isDebugEnabled()) logger.debug("Used default root priority 'NOTSET'.");
        } else {
            if (logger.isDebugEnabled()) logger.debug("Found root priority '"+rootPriority+"'.");
        }
        rootCategory = new LoggerCategoryInfo("", "", rootPriority);
        Collection keys = new SortedList(logCategories.keySet());
        for (Object o : keys) {
            String key = (String) o;
            String value = (String) logCategories.get(key);
            rootCategory.addChild(key, value);
        }
    }

    @SuppressWarnings("UnusedDeclaration")
    public LoggerCategoryInfo getRootCategory() {
        return rootCategory;
    }

    public String getId() {
        return null;
    }

    protected void load(final String loadId) throws SCAGJspException {
    }

    protected void save() throws SCAGJspException {
    }

    private Map<String, String> getLogsFromMap(Map parameters){
        Map<String, String> cats = new HashMap<String, String>();

        for (Object o : parameters.entrySet()) {
            Map.Entry entry = (Map.Entry) o;
            String paramName = (String) entry.getKey();
            if (paramName.startsWith(catParamNamePrefix)) {
                cats.put(paramName.substring(catParamNamePrefix.length()), getParamValue(entry.getValue()));
                logger.info("Logging.getLogsFromMap() cat param: " + paramName + "=" + getParamValue(entry.getValue()));
            } else {
                logger.warn("Logging.getLogsFromMap() WRONG param: " + paramName + ":=" + getParamValue(entry.getValue()));
            }
        }
        return cats;
    }

    protected void apply(Map parameters) throws SCAGJspException {
        logger.info( "Logging.apply() start" );
        Map<String, String> cats = getLogsFromMap( parameters );
        try {
            logger.info( "Logging.apply() setLogCategories" );

            appContext.getScag().setLogCategories(cats);
            cats = appContext.getScag().getLogCategories();
            parseMap( cats );

        } catch (SibincoException e) {
            logger.error( "Logging.apply():SibincoException" );
            if( e instanceof StatusDisconnectedException ){
                setRunning(false);
                logger.error( "Logging.apply():Disconnected" );
                readFLF( true );
            }
            throw new SCAGJspException(Constants.errors.logging.COULDNT_SET_LOGCATS, e);
        }
    }

    protected void savePermanent(Map parameters) throws SCAGJspException {
        Map<String, String> cats = getLogsFromMap(parameters);
        try {
            logger.info("Logging.savePermanent() storeLogConfig()");
            appContext.getScag().storeLogCategories(cats);
            logger.info("Logging.savePermanent() getLogCategories()");
            cats = appContext.getScag().getLogCategories(); //???
        } catch (SibincoException e) {
            logger.error( "Logging.savePermanent() SibincoException:can not store Scag logging");
            if( e instanceof StatusDisconnectedException ){
                try {
                    setRunning(false);
                    logger.error( "Logging.savePermanent() StatusDisconnectedException store Scag logging to file");
                    appContext.getLoggingManager().writeToLog(cats);
                } catch (SibincoException e1) {
                    logger.error( "Logging.savePermanent() SibincoException while writeToLog(). can not write Scag logging to file");
                }
            }
            throw new SCAGJspException(Constants.errors.logging.COULDNT_STORE_LOGCATS_FILE_WRITE, e);
        }finally{
            parseMap( cats );
        }
    }

    private String getParamValue(Object value) {
        if (value instanceof String)
            return (String) value;
        else if (value instanceof String[]) {
            String result = "";
            final String[] values = (String[]) value;

            for (String s : values) result += s;

            return result;
        } else
            return value.toString();
    }

    public String[] getPriorities() {
        return priorities;
    }

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    private void getLoggerCategoryInfo(LoggerCategoryInfo rootCategory, Map<String,LoggerCategoryInfo> map) {
        map.put(rootCategory.getFullName(), rootCategory);
        if (rootCategory.isHasChilds()) {
            logger.error( "Logging.getLoggerCategoryInfo() rootCategory=" + rootCategory.getFullName() + "|" + rootCategory.getName() );
            for (Object o : rootCategory.getChilds().values()) {
                LoggerCategoryInfo child = (LoggerCategoryInfo) o;
                map.put(child.getFullName(), child);
                if (logger.isDebugEnabled()) logger.debug("put child full name --> child: "+child.getFullName()+" --> "+child);
                if (child.isHasChilds()) getLoggerCategoryInfo(child, map);
            }
        }
    }

    @SuppressWarnings("UnusedDeclaration")
    public Map getFullNameToCatInfo() {
      return fullNameToCatInfo;
    }

    public String getMbSavePermanent() {
        return mbSavePermanent;
    }

    @SuppressWarnings("UnusedDeclaration")
    public void setMbSavePermanent(String mbSavePermanent) {
        this.mbSavePermanent = mbSavePermanent;
    }

    public String getMbRead() {
        return mbRead;
    }

    @SuppressWarnings("UnusedDeclaration")
    public void setMbRead(String mbRead) {
        this.mbRead = mbRead;
    }

    public boolean isRunning() {
        return running;
    }

    public void setRunning(boolean running) {
        this.running = running;
    }

    private String mbApply;

    public String getMbApply() {
        return mbApply;
    }

    @SuppressWarnings("UnusedDeclaration")
    public void setMbApply(String mbApply) {
        this.mbApply = mbApply;
    }
}
