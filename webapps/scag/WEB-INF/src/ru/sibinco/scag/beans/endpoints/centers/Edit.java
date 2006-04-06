/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Iterator;
import java.util.Map;
import java.util.ArrayList;
import java.security.Principal;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:10:30
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    public static final long ALL_PROVIDERS = -1;
    private String id = null;
    private String password = null;
    private int timeout = 0;
    private byte mode = Svc.MODE_TRX;
    private String host = null;
    private int port = 0;
    private String altHost = null;
    private int altPort = 0;
    private boolean enabled = false;
    private String bindSystemId;
    private String bindPassword;
    protected long providerId = -1;
    protected long transportId = 1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    private boolean administrator = false;
    private int uid = -1;
    private String addressRange = "";
    private long userProviderId = ALL_PROVIDERS;

    private void init() throws SCAGJspException {
        SCAGAppContext appContext = getAppContext();
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");

        userProviderId = user.getProviderId();
        administrator = (userProviderId == ALL_PROVIDERS);
        if (administrator) {
            Map providers = appContext.getProviderManager().getProviders();
            ArrayList ids = new ArrayList(100);
            ArrayList names = new ArrayList(100);
            for (Iterator i = providers.values().iterator(); i.hasNext();) {
                Object obj = i.next();
                if (obj != null && obj instanceof Provider) {
                    Provider provider = (Provider) obj;
                    ids.add(Long.toString(provider.getId()));
                    names.add(provider.getName());
                }
            }

            providerIds = (String[]) (ids.toArray(new String[0]));
            providerNames = (String[]) (names.toArray(new String[0]));
        } else {
            setProviderId(userProviderId);
            Object obj = appContext.getProviderManager().getProviders().get(new Long(userProviderId));
            if (obj == null || !(obj instanceof Provider))
                throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND,
                        "Failed to locate provider for id=" + userProviderId);
            providerName = ((Provider) obj).getName();
        }
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        this.init();
    }


    protected void load(String loadId) throws SCAGJspException {
        final Center center = (Center) appContext.getSmppManager().getCenters().get(loadId);

        if (null == center)
            throw new SCAGJspException(Constants.errors.smscs.SMSC_NOT_FOUND, loadId);

        this.id = center.getId();
        this.timeout = center.getTimeout();
        this.mode = center.getMode();
        this.host = center.getHost();
        this.port = center.getPort();
        this.altHost = center.getAltHost();
        this.altPort = center.getAltPort();
        this.enabled = center.isEnabled();
        this.providerId = -1;//center.getProvider().getId();
        this.uid = center.getUid();
        this.bindSystemId = center.getBindSystemId();
        this.bindPassword = center.getBindPassword();
        this.addressRange = center.getAddressRange();
    }

    protected void save() throws SCAGJspException {
        if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
            throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);

        if (null == password)
            password = "";

        final Provider providerObj = null;//(Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));
        final Map centers = appContext.getSmppManager().getCenters();
        if (centers.containsKey(id) && (isAdd() || !id.equals(getEditId())))
            throw new SCAGJspException(Constants.errors.sme.SME_ALREADY_EXISTS, id);
        Center oldCenter = null;
        if (!isAdd()) {
            oldCenter = (Center) centers.get(getEditId());
        }
        centers.remove(getEditId());
        final Center center;
        if (altHost == null || altHost.trim().length() == 0) altHost = "";
        if (bindPassword == null || bindPassword.trim().length() == 0) bindPassword = "";
        center = new Center(id, timeout, mode, host, port, altHost, altPort,
                enabled, providerObj, uid, bindSystemId, (bindPassword == null) ? "" : bindPassword, addressRange);
        centers.put(id, center);

        final Scag scag = appContext.getScag();
        try {
            if (isAdd()) {
                center.setUid(getLastUid());
                appContext.getSmppManager().setLastUsedId(center.getUid());
                if (center.isEnabled()) {
                    scag.addCenter(center);
                }

            } else {
                if (oldCenter.isEnabled() == center.isEnabled()) {
                    if (isEnabled())
                        scag.updateCenter(center);
                } else {
                    if (center.isEnabled()) {
                        scag.addCenter(center);
                    } else {
                        scag.deleteCenter(center);
                    }
                }
            }
        } catch (SibincoException e) {
            if (Proxy.STATUS_CONNECTED == scag.getStatus()) {
                if (isAdd()) centers.remove(id);
                logger.error("Couldn't applay Centers " + id + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, id, e);
            }
        } finally {
            oldCenter = null;
            try {
                appContext.getSmppManager().store();
            } catch (SibincoException e) {
                logger.error("Couldn't store smes ", e);
            }
        }
        throw new DoneException();
    }

    private int getLastUid() {
        uid = appContext.getSmppManager().getLastUsedId();
        uid++;
        return uid;
    }


    public String[] getSmes() {
        final SortedList smes = new SortedList(appContext.getSmppManager().getSvcs().keySet());
        for (Iterator i = appContext.getSmppManager().getSvcs().keySet().iterator(); i.hasNext();) {
            final String smscId = (String) i.next();
            if (!smscId.equals(id))
                smes.remove(smscId);
        }
        return (String[]) smes.toArray(new String[0]);
    }

    public String getId() {
        return id;
    }

    public void setId(final String id) {
        this.id = id;
    }

    public String getHost() {
        return host;
    }

    public void setHost(final String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(final int port) {
        this.port = port;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getAltHost() {
        return altHost;
    }

    public void setAltHost(final String altHost) {
        this.altHost = altHost;
    }

    public int getAltPort() {
        return altPort;
    }

    public void setAltPort(final int altPort) {
        this.altPort = altPort;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public String[] getTransportIds() {
        return Transport.transportIds;
    }

    public String[] getTransportTitles() {
        return Transport.transportTitles;
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(final long providerId) {
        this.providerId = providerId;
    }

    public String getProviderName() {
        return providerName;
    }

    public String[] getProviderIds() {
        return providerIds;
    }

    public boolean isAdministrator() {
        return administrator;
    }

    public String[] getProviderNames() {
        return providerNames;
    }

    public int getUid() {
        return uid;
    }

    public void setUid(final int uid) {
        this.uid = uid;
    }

    public String getBindSystemId() {
        return bindSystemId;
    }

    public void setBindSystemId(final String bindSystemId) {
        this.bindSystemId = bindSystemId;
    }

    public String getBindPassword() {
        return bindPassword;
    }

    public void setBindPassword(String bindPassword) {
        this.bindPassword = bindPassword;
    }

    public String getAddressRange() {
        return addressRange;
    }

    public void setAddressRange(String addressRange) {
        this.addressRange = addressRange;
    }

}