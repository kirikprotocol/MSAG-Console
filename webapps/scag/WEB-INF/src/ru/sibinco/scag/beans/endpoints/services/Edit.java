/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.services;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;
import java.util.ArrayList;
import java.util.Iterator;
import java.security.Principal;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:59:36
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    public static final long ALL_PROVIDERS = -1;
    protected String id = null;
    protected byte type = Svc.SMPP;
    protected String password = null;
    protected int timeout = 0;
    protected boolean enabled = false;
    protected byte mode = Svc.MODE_TRX;
    protected long providerId = -1;
    protected long transportId = 1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    private boolean administrator = false;
    private long userProviderId = ALL_PROVIDERS;


    public String getId() {
        return id;
    }

    private void init() throws SCAGJspException {

        SCAGAppContext context = getAppContext();
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) context.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");

        userProviderId = user.getProviderId();
        administrator = (userProviderId == ALL_PROVIDERS);
        if (administrator) {
            Map providers = context.getProviderManager().getProviders();
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
            Object obj = context.getProviderManager().getProviders().get(new Long(userProviderId));
            if (obj == null || !(obj instanceof Provider))
                throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND,
                        "Failed to locate provider for id=" + userProviderId);
            providerName = ((Provider) obj).getName();
        }
    }

    protected void load(String loadId) throws SCAGJspException {
        final Svc svc = (Svc) appContext.getSmppManager().getSvcs().get(loadId);

        if (null == svc)
            throw new SCAGJspException(Constants.errors.sme.SME_NOT_FOUND, loadId);

        this.id = svc.getId();
        this.mode = svc.getMode();
        this.password = svc.getPassword();
        this.providerId = svc.getProvider().getId();
        this.timeout = svc.getTimeout();
        this.enabled = svc.isEnabled();
    }

    protected void save() throws SCAGJspException {
        if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
            throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);

        if (null == password)
            password = "";
        final Provider providerObj = (Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));
        final Map svcs = appContext.getSmppManager().getSvcs();
        if (svcs.containsKey(id) && (isAdd() || !id.equals(getEditId())))
            throw new SCAGJspException(Constants.errors.sme.SME_ALREADY_EXISTS, id);
        Svc oldSvc = null;
        if (!isAdd()) {
            oldSvc = (Svc) svcs.get(getEditId());
        }
        svcs.remove(getEditId());
        final Svc svc;
        svc = new Svc(id, password, timeout, enabled, mode, providerObj);
        svcs.put(id, svc);

        final Gateway gateway = appContext.getGateway();
        try {
            if (isAdd()) {
                if (svc.isEnabled()) {
                    gateway.addSvc(svc);
                }
            } else {
                if ((oldSvc.isEnabled() == svc.isEnabled())) {
                    if(isEnabled())
                    gateway.updateSvcInfo(svc);
                } else {
                    if (svc.isEnabled()) {
                        gateway.addSvc(svc);
                    } else {
                        gateway.deleteSvc(svc.getId());
                    }
                }
            }
            oldSvc = null;
            appContext.getSmppManager().store();
        } catch (SibincoException e) {
            e.printStackTrace();
            if (Proxy.StatusConnected == gateway.getStatus()) {
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, id, e);
            }
        }
        throw new DoneException();
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        this.init();
    }

    public void setId(String id) {
        this.id = id;
    }


    public byte getType() {
        return type;
    }

    public void setType(final byte type) {
        this.type = type;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }


    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(final long providerId) {
        this.providerId = providerId;
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
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

    public String[] getTransportIds() {
        return Transport.transportIds;
    }

    public String[] getTransportTitles() {
        return Transport.transportTitles;
    }
}

