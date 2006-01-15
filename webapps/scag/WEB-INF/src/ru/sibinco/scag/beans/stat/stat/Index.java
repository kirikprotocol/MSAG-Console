package ru.sibinco.scag.beans.stat.stat;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.backend.stat.stat.StatQuery;
import ru.sibinco.scag.backend.stat.stat.Statistics;
import ru.sibinco.scag.backend.stat.stat.Stat;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;
import java.security.Principal;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.08.2004
 * Time: 14:32:38
 * To change this template use File | Settings | File Templates.
 */
public class Index extends SCAGBean {
    private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
    private static final String ALL_PROVIDERS = "ALL PROVIDERS";

    private String mbQuery = null;

    private StatQuery query = new StatQuery();
    private Statistics statistics = null;
    private Statistics httpStatistics = null;

    private boolean administrator = false;
    private long userProviderId = StatQuery.ALL_PROVIDERS;
    protected long transportId = 1;
    private String providerName = null;
    private String[] providerIds = null;
    private String[] providerNames = null;
    private String transport = null;

    private void init() throws SCAGJspException {
        SCAGAppContext context = getAppContext();
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(
                    Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) context.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(
                    Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");


        query.setTransport(getTransportId());
        userProviderId = user.getProviderId();
        administrator = (userProviderId == StatQuery.ALL_PROVIDERS);
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
            ids.add(0, Long.toString(StatQuery.ALL_PROVIDERS));
            names.add(0, ALL_PROVIDERS); // TODO: sort names ?
            providerIds = (String[]) (ids.toArray(new String[0]));
            providerNames = (String[]) (names.toArray(new String[0]));
        } else {
            query.setProviderId(userProviderId);
            Object obj = context.getProviderManager().getProviders().get(new Long(userProviderId));
            if (obj == null || !(obj instanceof Provider))
                throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND,
                        "Failed to locate provider for id=" + userProviderId);
            providerName = ((Provider) obj).getName();
        }
    }

    public void process(HttpServletRequest request, HttpServletResponse response)
            throws SCAGJspException {
        super.process(request, response);
        this.init();

        if (mbQuery != null) {
            try {
                if (userProviderId != StatQuery.ALL_PROVIDERS && userProviderId != query.getProviderId())
                    throw new Exception("Permission denied for user '" + providerName +
                            "' to access other providers's statistics");
                Stat stat = Stat.getInstance(appContext.getGwConfig());
                if (query.getTransport() == Transport.SMPP_TRANSPORT_ID) {
                    statistics = stat.getStatistics(query);
                } else if (query.getTransport() == Transport.HTTP_TRANSPORT_ID) {
                    httpStatistics = stat.getStatistics(query);
                }
            } catch (Exception exc) {
                statistics = null;
                mbQuery = null;
                throw new SCAGJspException(Constants.errors.stat.GET_STATISTICS_FAILED, exc);
            }
        } else if (!query.isFromDateEnabled()) {
            query.setFromDate(Functions.truncateTime(new Date()));
            query.setFromDateEnabled(true);
        }
        mbQuery = null;
    }

    public boolean isAdministrator() {
        return administrator;
    }

    public long getProviderId() {
        return query.getProviderId();
    }

    public void setProviderId(long providerId) {
        query.setProviderId(providerId);
    }

    public String[] getProviderIds() {
        return providerIds;
    }

    public String[] getProviderNames() {
        return providerNames;
    }

    public String getProviderName() {
        return providerName;
    }

    public String getTransport() {
        return transport;
    }

    private Date convertStringToDate(String date) {
        Date converted = new Date();
        try {
            SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
            converted = formatter.parse(date);
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return converted;
    }

    private String convertDateToString(Date date) {
        SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
        return formatter.format(date);
    }

    public String getFromDate() {
        return (query.isFromDateEnabled()) ? convertDateToString(query.getFromDate()) : "";
    }

    public void setFromDate(String fromDate) {
        if (fromDate != null && fromDate.trim().length() > 0) {
            query.setFromDate(convertStringToDate(fromDate));
            query.setFromDateEnabled(true);
        } else {
            query.setFromDateEnabled(false);
        }
    }

    public String getTillDate() {
        return (query.isTillDateEnabled()) ? convertDateToString(query.getTillDate()) : "";
    }

    public void setTillDate(String tillDate) {
        if (tillDate != null && tillDate.trim().length() > 0) {
            query.setTillDate(convertStringToDate(tillDate));
            query.setTillDateEnabled(true);
        } else {
            query.setTillDateEnabled(false);
        }
    }

    public Statistics getStatistics() {
        return statistics;
    }

    public Statistics getHttpStatistics() {
        return httpStatistics;
    }

    public String getMbQuery() {
        return mbQuery;
    }

    public void setMbQuery(String mbQuery) {
        this.mbQuery = mbQuery;
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

}
