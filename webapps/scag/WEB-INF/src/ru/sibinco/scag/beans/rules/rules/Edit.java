/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.rules.rules;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.users.User;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.Map;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 26.09.2005
 * Time: 16:31:47
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    public static final long ALL_PROVIDERS = -1;
    private String mbNext = null;
    protected long transportId = 1;
    private String name;
    private long ruleId = -1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    protected long providerId = -1;
    private long userProviderId = -1;
    private boolean administrator = false;


    public String getId() {
        return null;
    }

    private void init() throws SCAGJspException {
        SCAGAppContext appContext = getAppContext();
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to  obtain user principal(s)");
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
            providerIds = (String[]) (ids.toArray(new String[ids.size()]));
            providerNames = (String[]) (names.toArray(new String[names.size()]));
        } else {
            setProviderId(userProviderId);
            Object obj = appContext.getProviderManager().getProviders().get(new Long(userProviderId));
            if (obj == null || !(obj instanceof Provider))
                throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND,
                        "Failed to locate provider for id=" + userProviderId);
            providerName = ((Provider) obj).getName();
        }

    }

    protected void load(final String loadId) throws SCAGJspException {

        final Rule rule = (Rule) appContext.getRuleManager().getRules().get(Long.decode(loadId));
    if (null == rule)
      throw new SCAGJspException(Constants.errors.sme.SME_NOT_FOUND, loadId);
        else
        System.out.println("!!!!!!!!!!!!!!!!");
    }

    protected void save() throws SCAGJspException {
        ruleId = getRuleId();
        if ( 0 >= ruleId || !isAdd())
          error("Rule id not specified",null);
        if (null == name || 0 == name.length())
            error("Rule name not found",null);
    String[] titles=getTransportTitles();
    int id=(int)transportId;
    String transport=titles[id];
    String[] schemaNames=getSchemaNames();
    String schema=schemaNames[id];
    String header="<scag:rule xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'\n"+
                  "xsi:schemaLocation='http://www.sibinco.com/SCAG xsd/'"+schema+"\n"+
                 "xmlns:scag='http://www.sibinco.com/SCAG'\n"+
                 "transport='"+transport+"' id='"+ruleId+"' provider='"+providerId+"' name='"+name+"'>";

    String footer="</scag:rule>";
    Provider provider=(Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));
    if (provider==null)  error("Provider with id= "+providerId+" not found",null);
    Rule newRule=new Rule(new Long(ruleId),name,"",provider,transport);
    appContext.getRuleManager().setNewRule(newRule);
    appContext.getRuleManager().setOpenNewRule(true);
    throw new DoneException();
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        this.init();
    }

    public String getMbNext() {
        return mbNext;
    }

    public void setMbNext(String mbNext) {
        this.mbNext = mbNext;
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }

    public String[] getTransportIds() {
        return Transport.transportIds;
    }

    public String[] getTransportTitles() {
        return Transport.transportTitles;
    }
      public String[] getSchemaNames() {
        return Transport.schemaNames;
    }
    public long getRuleId() {
        ruleId = appContext.getRuleManager().getLastRuleId();
        ruleId ++;
        return ruleId;
    }

    public String getProviderName() {
        return providerName;
    }

    public String[] getProviderIds() {
        return providerIds;
    }

    public String[] getProviderNames() {
        return providerNames;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(long providerId) {
        this.providerId = providerId;
    }

    public boolean isAdministrator() {
        return administrator;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }
}