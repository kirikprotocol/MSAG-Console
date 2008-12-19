/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:10:30
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean
{
    public static final long ALL_PROVIDERS = -1;
    private String id = null;
    private String password = null;
    private int timeout = 0;
    private byte mode = Svc.MODE_TRX;
    private String host = null;
    private int port = 0;
    private String altHost = null;
    private int altPort = 0;
    private boolean enabled;
    private boolean snmptracking;
    private String bindSystemId;
    private String bindPassword;
    private String systemType;
    protected long providerId = -1;
    protected long transportId = 1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    private boolean administrator = false;
    private int uid = -1;
    private String addressRange = "";
    private long userProviderId = ALL_PROVIDERS;
    private Principal userPrincipal = null;
    private int inQueueLimit = 0;
    private int outQueueLimit = 0;
    private int maxSmsPerSec = 0;

    private void init() throws SCAGJspException {
        SCAGAppContext appContext = getAppContext();
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
        if(isAdd()){
            enabled = true;
        }
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        userPrincipal = request.getUserPrincipal();
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
        this.snmptracking = center.isSnmptracking();
        this.providerId = -1;//center.getProvider().getId();
        this.uid = center.getUid();
        this.bindSystemId = center.getBindSystemId();
        this.bindPassword = center.getBindPassword();
        this.systemType = center.getSystemType();
        this.addressRange = center.getAddressRange();
        this.inQueueLimit = center.getInQueueLimit();
        this.outQueueLimit = center.getOutQueueLimit();
        this.maxSmsPerSec = center.getMaxSmsPerSec();
    }

    protected void save() throws SCAGJspException {
        if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
            throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);
        if( !validateString(id, VALIDATION_TYPE_ID) ){
            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID, "ID");
        }
        if( !validateString(bindSystemId, VALIDATION_TYPE_ID) ){
            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID, "bind system Id");
        }
        if (null == password) password = "";

        final Provider providerObj = null;//(Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));
        final Map centers = appContext.getSmppManager().getCenters();
        if (centers.containsKey(id) && (isAdd() || !id.equals(getEditId())))
            throw new SCAGJspException(Constants.errors.sme.SME_ALREADY_EXISTS, id);
        if (altHost == null || getAltHost().length() == 0) altHost = "";
        if (bindPassword == null || getBindPassword().length() == 0) bindPassword = "";
        if (systemType == null || getSystemType().length() == 0) systemType = "";
//        center = new Center(getId(), timeout, mode, getHost(), port, getAltHost(), altPort,
//                enabled, providerObj, uid, getBindSystemId(), getBindPassword(), getSystemType(), getAddressRange());
//        center = new Center(getId(), timeout, mode, getHost(), port, getAltHost(), altPort,
//                enabled, providerObj, uid, getBindSystemId(), getBindPassword(), getSystemType(),
//                getAddressRange(), inQueueLimit, maxSmsPerSec);

        Center oldCenter = null;
        final Center center;
        if (!isAdd()) {
            oldCenter = (Center) centers.get(getEditId());
            center = new Center(getId(), timeout, mode, getHost(), port, getAltHost(), altPort,
                    enabled, providerObj, uid, getBindSystemId(), getBindPassword(), getSystemType(),
                    getAddressRange(), inQueueLimit, outQueueLimit, maxSmsPerSec, oldCenter.getMetaGroup(), snmptracking);
        }else{
            center = new Center(getId(), timeout, mode, getHost(), port, getAltHost(), altPort,
                    enabled, providerObj, uid, getBindSystemId(), getBindPassword(), getSystemType(),
                    getAddressRange(), inQueueLimit, outQueueLimit, maxSmsPerSec, "", snmptracking);
        }
        centers.remove(getEditId());
        centers.put(id, center);

        appContext.getSmppManager().createUpdateCenter(getLoginedPrincipal().getName(),
                isAdd(), isEnabled(), center, appContext, oldCenter);
        throw new DoneException();
    }

    private final static int VALIDATION_TYPE_ID = 0;

    public boolean validateString( String string, int type )
    {
        switch (type){
            case VALIDATION_TYPE_ID:
                return Pattern.matches( "[a-zA-Z_0-9]{1,15}", string );
            default:
                return true;
        }
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
        if(id != null) id.trim();
        return id;
    }

    public void setId(final String id) {
        this.id = id;
    }

    public String getHost() {
        if(host != null) host.trim();
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
        if(password != null)password.trim();
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getAltHost() {
        if(altHost != null)altHost.trim();
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

    public boolean isSnmptracking() {
        return snmptracking;
    }

    public void setSnmptracking(final boolean snmptracking) {
        this.snmptracking = snmptracking;
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
        if(providerName != null)providerName.trim();
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
        if(bindSystemId != null)bindSystemId.trim();
        return bindSystemId;
    }

    public void setBindSystemId(final String bindSystemId) {
        this.bindSystemId = bindSystemId;
    }

    public String getBindPassword() {
        if(bindPassword != null)bindPassword.trim();
        return bindPassword;
    }

    public void setBindPassword(String bindPassword) {
        this.bindPassword = bindPassword;
    }

    public String getSystemType() {
       if(systemType != null) systemType.trim();
       return systemType;
    }

    public void setSystemType(String systemType) {
        this.systemType = systemType;
    }

    public String getAddressRange() {
        if(addressRange != null)addressRange.trim();
        return addressRange;
    }

    public void setAddressRange(String addressRange) {
        this.addressRange = addressRange;
    }
    public void setInQueueLimit(int inQueueLimit) {
        this.inQueueLimit = inQueueLimit;
    }

    public void setOutQueueLimit(int outQueueLimit) {
        this.outQueueLimit = outQueueLimit;
    }

    public void setMaxSmsPerSec(int maxSmsPerSec) {
        this.maxSmsPerSec = maxSmsPerSec;
    }

    public int getInQueueLimit() {
        return inQueueLimit;
    }

    public int getOutQueueLimit() {
        return outQueueLimit;
    }

    public int getMaxSmsPerSec() {
        return maxSmsPerSec;
    }
}