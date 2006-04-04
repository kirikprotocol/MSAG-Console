package ru.sibinco.scag.beans.gw.config;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Statuses;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;
import java.io.IOException;


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

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        requestParams = request.getParameterMap();
        super.process(request, response);

        if (null != mbStart)
            start();
        else if (null != mbStop)
            stop();
    }

    public String getId() {
        return null;
    }

    protected void load(String loadId) throws SCAGJspException {
        for (Iterator i = requestParams.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            if (entry.getKey() instanceof String) {
                String key = (String) entry.getKey();
                if (key.startsWith(COLLAPSING_TREE_PARAM_PREFIX)) {
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

    protected void save() throws SCAGJspException {
        final Config gwConfig = appContext.getGwConfig();
        final Statuses statuses = appContext.getStatuses();
        for (Iterator i = params.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            final Object parameter = gwConfig.getParameter((String) entry.getKey());
            if (parameter != null) {
                if (parameter instanceof String)
                    gwConfig.setString((String) entry.getKey(), (String) entry.getValue());
                else if (parameter instanceof Integer || parameter instanceof Long)
                    try {
                        gwConfig.setInt((String) entry.getKey(), Integer.parseInt(((String) entry.getValue()).trim()));
                    } catch (NumberFormatException e) {
                        throw new SCAGJspException(Constants.errors.config.INVALID_INTEGER, (String) entry.getValue());
                    }
                else if (parameter instanceof Boolean)
                    gwConfig.setBool((String) entry.getKey(), Boolean.valueOf((String) entry.getValue()).booleanValue());
                else
                    gwConfig.setString((String) entry.getKey(), ((String) entry.getValue()).trim());
            } else
                gwConfig.setString((String) entry.getKey(), ((String) entry.getValue()).trim());
            statuses.setConfigChanged(true);
            this.configChanged = true;
        }

        try {
            appContext.getGwConfig().save();
            appContext.getStatuses().setConfigChanged(false);
            started = true;
            stopped = true;
        } catch (Config.WrongParamTypeException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
        throw new DoneException();
    }

    private void applyConfig() throws SCAGJspException {
        try {

            try {
                appContext.getScag().apply("config");
            } catch (SibincoException e) {
                if (Proxy.STATUS_CONNECTED == appContext.getScag().getStatus()) {
                    logger.debug("Couldn't apply config", e);
                    throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
                }
            }

        } catch (SibincoException e) {
            logger.debug("Couldn't apply config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
        }
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

}
