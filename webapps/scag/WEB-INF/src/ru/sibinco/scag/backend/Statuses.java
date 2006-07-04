package ru.sibinco.scag.backend;


/**
 * Created by igork Date: 17.06.2004 Time: 18:24:03
 */
public class Statuses {

    private boolean configChanged = false;
    private boolean routesChanged = false;
    private boolean usersChanged = false;
    private boolean smscsChanged = false;
    private boolean billingChanged = false;
    private boolean aliasesChanged = false;
    private boolean httpRoutesLoaded = false;
    private boolean smppRoutesLoaded = false;

    public boolean isConfigChanged() {
        return configChanged;
    }

    public void setConfigChanged(final boolean configChanged) {
        this.configChanged = configChanged;
    }

    public boolean isRoutesChanged() {
        return routesChanged;
    }

    public void setRoutesChanged(final boolean routesChanged) {
        this.routesChanged = routesChanged;
    }

    public boolean isUsersChanged() {
        return usersChanged;
    }

    public void setUsersChanged(final boolean usersChanged) {
        this.usersChanged = usersChanged;
    }

    public boolean isSmscsChanged() {
        return smscsChanged;
    }

    public void setSmscsChanged(final boolean smscsChanged) {
        this.smscsChanged = smscsChanged;
    }

    public boolean isBillingChanged() {
        return billingChanged;
    }

    public void setBillingChanged(final boolean billingChanged) {
        this.billingChanged = billingChanged;
    }

    public boolean isAliasesChanged() {
        return aliasesChanged;
    }

    public void setAliasesChanged(boolean aliasesChanged) {
        this.aliasesChanged = aliasesChanged;
    }

    public boolean isHttpRoutesLoaded() {
        return httpRoutesLoaded;
    }

    public void setHttpRoutesLoaded(boolean httpRoutesLoaded) {
        this.httpRoutesLoaded = httpRoutesLoaded;
    }

    public boolean isSmppRoutesLoaded() {
        return smppRoutesLoaded;
    }

    public void setSmppRoutesLoaded(boolean smppRoutesLoaded) {
        this.smppRoutesLoaded = smppRoutesLoaded;
    }
}
