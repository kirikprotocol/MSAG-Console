/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.gw.logging;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.util.Comparator_CaseInsensitive;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;

/**
 * The <code>Logging</code> class represents
 * <p><p/>
 * Date: 21.07.2006
 * Time: 11:50:29
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Logging extends EditBean {
    public static final String catParamNamePrefix = "category_";
    private LoggerCategoryInfo rootCategory;
    private String[] priorities = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "NOTSET"};
    private String category;
    private String mbSavePermanent = null;
    private boolean running = true;
    private Map fullNameToCatInfo = new TreeMap();

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        if (getMbSave() != null) {
            save(request.getParameterMap());
        }
        if (getMbSavePermanent() != null) {
            savePermanent(request.getParameterMap());
        }
        init();
    }

    public class LoggerCategoryInfo implements Comparable {
        private String name;
        private String fullName;
        private String priority;
        private Map childs;

        public LoggerCategoryInfo(String name, String fullName, String priority) {
            this.name = name;
            this.fullName = fullName;
            this.priority = priority;
            this.childs = new TreeMap(new Comparator_CaseInsensitive());
        }

        public void addChild(String childFullName, String childPriority) {
            logger.debug(new StringBuffer().append("Adding child isRoot:'" + isRoot() + "' : name=\"").append(fullName).
                    append("\" priority=").append(priority).append(", childName=\"").append(childFullName).
                    append("\" childPriority=").append(childPriority).toString());
            if (isRoot()) {
                final int dotPos = childFullName.indexOf('.');
                if (dotPos > 0) {
                    final String childName = childFullName.substring(0, dotPos);
                    LoggerCategoryInfo child = getOrCreateChild(childName, childName, "NOTSET");
                    child.addChild(childFullName, childPriority);
                } else {
                    childs.put(childFullName, new LoggerCategoryInfo(childFullName, childFullName, childPriority));
                }
            } else {
                if (childFullName.length() > fullName.length() && childFullName.startsWith(fullName) && childFullName.charAt(fullName.length()) == '.')
                {
                    final int beginIndex = fullName.length() + 1;
                    final int endIndex = childFullName.indexOf('.', beginIndex);
                    final String childName = endIndex > 0 ? childFullName.substring(beginIndex, endIndex) : childFullName.substring(beginIndex);
                    if (endIndex > 0) {
                        LoggerCategoryInfo child = getOrCreateChild(childName, childFullName.substring(0, endIndex), "NONSET");
                        child.addChild(childFullName, childPriority);
                    } else {
                        LoggerCategoryInfo child = getOrCreateChild(childName, childFullName, childPriority);
                        child.priority = childPriority;
                    }
                } else {
                    logger.debug("Incorrect LogerCategoryInfo.addChild algorithm");
                }
            }
        }

        private LoggerCategoryInfo getOrCreateChild(String childName, String childFullName, String childPriority) {
            LoggerCategoryInfo child = (LoggerCategoryInfo) childs.get(childName);
            if (child == null) {
                child = new LoggerCategoryInfo(childName, childFullName, childPriority);
                childs.put(childName, child);
            }
            return child;
        }

        public boolean isRoot() {
            return fullName == null || fullName.length() == 0;
        }


        public boolean hasChilds() {
            return !childs.isEmpty();
        }

        public boolean isHasChilds() {
            return !childs.isEmpty();
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

        public Map getChilds() {
            return childs;
        }

        public String toString() {
            return " # name : "+ name +
                   " | fullName : " + fullName +
                   " | priority : " + priority +
                   " | childs : "+ childs.values() + "#\n";
        }
    }

    protected void init() {
        try {
            Map logCategories = appContext.getScag().getLogCategories();
            String rootPriority = (String) logCategories.remove("");
            if (rootPriority == null) rootPriority = "NOTSET";
            rootCategory = new LoggerCategoryInfo("", "", rootPriority);
            Collection keys = new SortedList(logCategories.keySet());
            for (Iterator iterator = keys.iterator(); iterator.hasNext();) {
                String key = (String) iterator.next();
                String value = (String) logCategories.get(key);
                rootCategory.addChild(key, value);
            }

        } catch (SibincoException e) {
            logger.error( "init:Exception while init()" );

            rootCategory = new LoggerCategoryInfo("", "", "NOTSET");
            setRunning(false);
        }
        getLoggerCategoryInfos(rootCategory, fullNameToCatInfo);
    }

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

    protected void save(Map parameters) throws SCAGJspException {
        Map cats = new HashMap();
        for (Iterator iterator = parameters.entrySet().iterator(); iterator.hasNext();) {
            Map.Entry entry = (Map.Entry) iterator.next();
            String paramName = (String) entry.getKey();
            if (paramName.startsWith(catParamNamePrefix)) {
                cats.put(paramName.substring(catParamNamePrefix.length()), getParamValue(entry.getValue()));
                logger.info("cat param: " + paramName + ":=" + getParamValue(entry.getValue()));
            } else {
                logger.info("WRONG param: " + paramName + ":=" + getParamValue(entry.getValue()));
            }

        }
        try {
            appContext.getScag().setLogCategories(cats);
        } catch (SibincoException e) {
            throw new SCAGJspException(Constants.errors.logging.COULDNT_SET_LOGCATS, e);
        }
    }

    protected void savePermanent(Map parameters) throws SCAGJspException {
        Map cats = new HashMap();
        for (Iterator iterator = parameters.entrySet().iterator(); iterator.hasNext();) {
            Map.Entry entry = (Map.Entry) iterator.next();
            String paramName = (String) entry.getKey();
            if (paramName.startsWith(catParamNamePrefix)) {
                cats.put(paramName.substring(catParamNamePrefix.length()), getParamValue(entry.getValue()));
                logger.info("cat param: " + paramName + ":=" + getParamValue(entry.getValue()));
            } else {
                logger.info("WRONG param: " + paramName + ":=" + getParamValue(entry.getValue()));
            }

        }
        try {
//            appContext.getScag().setLogCategories(cats);
            logger.error( "Logging:savePermanent():storeLogConfig()" );
            appContext.getScag().storeLogCategories(cats);
//            appContext.getLoggingManager().writeToLog(cats);
        } catch (SibincoException e) {
            logger.error( "Logging:savePermanent():can not store Scag logging");
            try {
                appContext.getLoggingManager().writeToLog(cats);
            } catch (SibincoException e1) {
                logger.error( "Logging:savePermanent():can not store Scag logging to file");
                e1.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
            }
            throw new SCAGJspException(Constants.errors.logging.COULDNT_SET_LOGCATS_FILE_WRITE, e);
        }
    }

    private String getParamValue(Object value) {
        if (value instanceof String)
            return (String) value;
        else if (value instanceof String[]) {
            String result = "";
            final String[] values = (String[]) value;
            for (int i = 0; i < values.length; i++) {
                result += values[i];
            }
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

    private void getLoggerCategoryInfos(LoggerCategoryInfo rootCategory, Map map) {
        map.put(rootCategory.getFullName(),rootCategory);
        logger.error( "Logging:getLoggerCategoryInfos:MAP=" + map.keySet().toString() );
        if (rootCategory.hasChilds()) {
            logger.error( "Logging:getLoggerCategoryInfos:rootCategory=" + rootCategory.getFullName() + "|" + rootCategory.getName() );
            logger.error( "Logging:getLoggerCategoryInfos:rootCategory.childs=" + rootCategory.getChilds().values().toString() );
            for (Iterator i = rootCategory.getChilds().values().iterator(); i.hasNext();) {
                LoggerCategoryInfo child = (Logging.LoggerCategoryInfo) i.next();
                logger.error( "Logging:getLoggerCategoryInfos:child=" + child.getFullName() + "|" +child.getName() );
                map.put(child.getFullName(),child);
                if (child.hasChilds()) {
                    logger.error( "Logging:getLoggerCategoryInfos:hasChilds" );
                    getLoggerCategoryInfos(child, map);
                }
            }
        }
    }

    public Map getFullNameToCatInfo() {
      return fullNameToCatInfo;
    }

    public String getMbSavePermanent() {
        return mbSavePermanent;
    }

    public void setMbSavePermanent(String mbSavePermanent) {
        this.mbSavePermanent = mbSavePermanent;
    }

    public boolean isRunning() {
        return running;
    }

    public void setRunning(boolean running) {
        this.running = running;
    }
}
