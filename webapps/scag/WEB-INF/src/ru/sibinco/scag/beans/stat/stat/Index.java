package ru.sibinco.scag.beans.stat.stat;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.backend.stat.stat.StatQuery;
import ru.sibinco.scag.backend.stat.stat.Statistics;
import ru.sibinco.scag.backend.stat.stat.Stat;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;

public class Index extends SCAGBean {
    private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
    private static final String ALL_PROVIDERS = "ALL PROVIDERS";
    private static final String ALL_SERVICES = "ALL SERVICES";

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
    private String serviceName = null;
    private String[] serviceIds = null;
    private String[] serviceNames = null;
    private String transport = null;

    private void init() throws SCAGJspException {
        SCAGAppContext context = getAppContext();
        String userName = super.getUserName();
        if (userName == null)
            throw new SCAGJspException(
                    Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user name");
        User user = (User) context.getUserManager().getUsers().get(userName);
        if (user == null)
            throw new SCAGJspException(
                    Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userName + "'");

        query.setTransport(getTransportId());
        userProviderId = user.getProviderId();
        administrator = (userProviderId == StatQuery.ALL_PROVIDERS);
        if (administrator) {
            Map providers = context.getServiceProviderManager().getServiceProviders(); //context.getProviderManager().getProviders();
            ArrayList ids = new ArrayList(100);
            ArrayList names = new ArrayList(100);
            ArrayList serviceIds = new ArrayList(100);
            ArrayList serviceNames = new ArrayList(100);
            String prId;
            for (Iterator i = providers.values().iterator(); i.hasNext();) {
                Object obj = i.next();
                if (obj != null && obj instanceof ServiceProvider) {
                    ServiceProvider provider = (ServiceProvider) obj;
                    prId = provider.getId().toString();
                    if( context.getServiceProviderManager().checkForbiddenChars(provider.getName()) ){
                        names.add(provider.getName());
                        for(Iterator ii = provider.getServices().values().iterator();ii.hasNext();) {
                           Object obj1 = ii.next();
                           if (obj1 != null && obj1 instanceof Service) {
                             Service service = (Service) obj1;
                               if( context.getServiceProviderManager().checkForbiddenChars(service.getName()) ){
                                    serviceIds.add(service.getId().toString()+"//"+prId);
                                    serviceNames.add(service.getName());
                               }else{
                                   logger.error( "Service name contain forbidden character '" +
                                           Constants.FORBIDDEN_CHARACTER + "' in '" + service.getName() + "' " +
                                           "Provider name '" + provider.getName() + "'" );
                               }
                           }
                        }
                        ids.add(prId);
                    }else{
                        logger.error( "Provider name contain forbidden character '" +
                                           Constants.FORBIDDEN_CHARACTER + "' in '"+ provider.getName() + "'");
                    }
                }
            }
            ids.add(0, Long.toString(StatQuery.ALL_PROVIDERS));
            names.add(0, ALL_PROVIDERS); // TODO: sort names ?
            providerIds = (String[]) (ids.toArray(new String[0]));
            providerNames = (String[]) (names.toArray(new String[0]));
            serviceIds.add(0, Long.toString(StatQuery.ALL_SERVICES));
            serviceNames.add(0, ALL_SERVICES);
            this.serviceIds = (String[]) (serviceIds.toArray(new String[0]));
            this.serviceNames = (String[]) (serviceNames.toArray(new String[0]));
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
                Stat stat = Stat.getInstance(appContext);
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

    public String getServiceId() {
        return String.valueOf(query.getServiceId() + "//"+query.getProviderId());
    }

    public void setServiceId(String serviceId) {
        query.setServiceId(Long.valueOf(serviceId).longValue());
    }

    public String[] getServiceIds() {
        logger.info( "STAT:serviceIds=" + serviceIds );
        return serviceIds;
    }

    public String[] getServiceNames() {
        logger.info( "STAT:serviceNames=" + serviceNames );
        return serviceNames;
    }

    public String getServiceName() {
        return serviceName;
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
