/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.services;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.users.User;

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
public class Edit extends EditBean{

    public static final long ALL_PROVIDERS = -1;
    protected String id = null;
    protected int priority = 0;
    protected byte type = Svc.SMPP;
    protected int typeOfNumber = 0;
    protected int numberingPlan = 0;
    protected int interfaceVersion = 0;
    protected String systemType = null;
    protected String password = null;
    protected String addrRange = null;
    protected int timeout = 0;
    protected String receiptSchemeName = null;
    protected boolean disabled = false;
    protected byte mode = Svc.MODE_TRX;
    protected int proclimit = 0;
    protected boolean smsc = false;
    protected long providerId = -1;
    protected long transportId = 1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    private boolean administrator = false;
    private long userProviderId = ALL_PROVIDERS;


    public String getId() {
        return null;
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
        //To change body of implemented methods use File | Settings | File Templates.
    }

    protected void save() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        this.init();
    }

    public void setId(String id) {
        this.id = id;
    }

    public int getPriority() {
        return priority;
    }

    public void setPriority(final int priority) {
        this.priority = priority;
    }

    public byte getType() {
        return type;
    }

    public void setType(final byte type) {
        this.type = type;
    }

    public int getTypeOfNumber() {
        return typeOfNumber;
    }

    public void setTypeOfNumber(final int typeOfNumber) {
        this.typeOfNumber = typeOfNumber;
    }

    public int getNumberingPlan() {
        return numberingPlan;
    }

    public void setNumberingPlan(final int numberingPlan) {
        this.numberingPlan = numberingPlan;
    }

    public int getInterfaceVersion() {
        return interfaceVersion;
    }

    public void setInterfaceVersion(final int interfaceVersion) {
        this.interfaceVersion = interfaceVersion;
    }

    public String getSystemType() {
        return systemType;
    }

    public void setSystemType(final String systemType) {
        this.systemType = systemType;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getAddrRange() {
        return addrRange;
    }

    public void setAddrRange(final String addrRange) {
        this.addrRange = addrRange;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public String getReceiptSchemeName() {
        return receiptSchemeName;
    }

    public void setReceiptSchemeName(final String receiptSchemeName) {
        this.receiptSchemeName = receiptSchemeName;
    }

    public boolean isDisabled() {
        return disabled;
    }

    public void setDisabled(final boolean disabled) {
        this.disabled = disabled;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public int getProclimit() {
        return proclimit;
    }

    public void setProclimit(final int proclimit) {
        this.proclimit = proclimit;
    }

    public boolean isSmsc() {
        return smsc;
    }

    public void setSmsc(final boolean smsc) {
        this.smsc = smsc;
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

    public String[] getTranspotIds() {  //ToDo
        String[] transpotIds = {
            "1",  "2", "3"
        };
        return transpotIds;
    }

    public String[] getTranspotTitles() { //ToDo
        String[] transpotTitles = {
            "SMPP",  "WAP", "MMS"
        };
        return transpotTitles;
    }

}

