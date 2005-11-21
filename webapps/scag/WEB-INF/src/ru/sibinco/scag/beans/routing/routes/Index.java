package ru.sibinco.scag.beans.routing.routes;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Collection;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.Date;
import java.security.Principal;
import java.text.DateFormat;


/**
 * Created by igork Date: 19.04.2004 Time: 17:39:13
 */
public class Index extends TabledBeanImpl {

    private String mbApply;
    private String mbRestore;
    private String mbLoadSaved;
    private String mbLoad;
    private String mbSave;
    private String[] subj;
    private boolean routesChanged;
    private String changeByUser;
    private boolean currentUser;
    private boolean routesSaved = false;
    private boolean routesRestored = false;
    private String restoreDate = null;
    private boolean routesLoaded = false;


    protected Collection getDataSource() {
        return appContext.getScagRoutingManager().getRoutes().values();
    }


    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        super.process(request, response); // initializes bean values
        if (null != mbRestore)
            restoreRoutes();
        else if (null != mbApply)
            applyRoutes();
        else if (null != mbLoadSaved)
            restoreRoutes();
        else if (mbLoad != null)
            loadRoutes();
        else if (mbSave != null)
            saveRoutes();
        this.init();
        super.process(request, response); //refresh changes bean values (it's bug from class TabledBeanImpl) 

    }


    private void init() throws SCAGJspException {
        SCAGAppContext appContext = getAppContext();

        final Locale locale = new Locale(Functions.getLocaleEncoding());
        Calendar restoreCalendar = new GregorianCalendar(locale);
        DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM, locale);

        final Date restoreFileDate;
        try {
            restoreFileDate = appContext.getScagRoutingManager().getRestoreFileDate();
            if (restoreFileDate != null) {
                restoreCalendar.setTime(restoreFileDate);
                restoreDate = dateFormat.format(restoreFileDate);
            }
        } catch (SibincoException e) {
            e.printStackTrace();
        }


        routesRestored = appContext.getScagRoutingManager().isRoutesRestored();
        routesSaved = appContext.getScagRoutingManager().isRoutesSaved();
        if (appContext.getScagRoutingManager().hasSavedConfiguration()) {
            routesLoaded = appContext.getScagRoutingManager().isRoutesLoaded();

        }

        routesChanged = appContext.getScagRoutingManager().isRoutesChanged();
        if (routesChanged) {
            changeByUser = appContext.getScagRoutingManager().getChangedByUser();
            currentUser = getUser(appContext).getName().equals(changeByUser);
        }
    }

    private User getUser(SCAGAppContext appContext) throws SCAGJspException {
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        return user;
    }

    protected void delete() {
        appContext.getScagRoutingManager().getRoutes().keySet().removeAll(checkedSet);
        appContext.getStatuses().setRoutesChanged(true);
    }

    /**
     * This private method will be restory routes from route_.xml file
     * saved user's earlier
     *
     * @throws SCAGJspException - an throws exception if it will be occurred
     */
    private void restoreRoutes() throws SCAGJspException {
        try {

            appContext.getScagRoutingManager().restore();

            //set changed by user, it wil be blocked any chages for other users
            appContext.getScagRoutingManager().setChangedByUser(getUserName(appContext));

            appContext.getScagRoutingManager().setRoutesChanged(true);
            appContext.getScagRoutingManager().setRoutesRestored(true);
            appContext.getScagRoutingManager().setRoutesSaved(false);
            appContext.getScagRoutingManager().setRoutesLoaded(false);

        } catch (SibincoException e) {
            logger.debug("Couldn't restore routes", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_RESTORE_ROUTES, e);
        }

    }

    private void applyRoutes() throws SCAGJspException {
        try {
            appContext.getScagRoutingManager().apply();
            try {
                appContext.getGateway().apply("routes");
            } catch (SibincoException e) {
                if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
                    logger.debug("Couldn't apply routes", e);
                    throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_APPLY_ROUTES, e);
                }
            }
            appContext.getScagRoutingManager().setChangedByUser("");
            appContext.getScagRoutingManager().setRoutesChanged(false);
            init();
        } catch (SibincoException e) {
            logger.debug("Couldn't apply routes", e);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_APPLY_ROUTES, e);
        }
    }

    /**
     * This method provides to load applied configuration
     *
     * @throws SCAGJspException - an throws exception if it will
     *                          be occurred@throws SCAGJspException
     */
    protected void loadRoutes() throws SCAGJspException {

        try {
            appContext.getScagRoutingManager().load();
        } catch (SibincoException exc) {
            logger.debug("Couldn't load routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_LOAD_ROUTES, exc);
        }
        appContext.getScagRoutingManager().setRoutesChanged(false);
        appContext.getScagRoutingManager().setRoutesRestored(false);
        appContext.getScagRoutingManager().setRoutesLoaded(true);
        appContext.getScagRoutingManager().setRoutesSaved(false);
    }

    // saves temporal configuration
    protected void saveRoutes() throws SCAGJspException {
        try {
            appContext.getScagRoutingManager().save();
        } catch (SibincoException exc) {
            logger.debug("Couldn't save routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_SAVE_ROUTES, exc);
        }
        appContext.getScagRoutingManager().setRoutesSaved(true);
        appContext.getScagRoutingManager().setRoutesRestored(true);
    }


    public boolean isRoutesChanged() {
        return routesChanged;
    }

    public void setRoutesChanged(boolean routesChanged) {
        this.routesChanged = routesChanged;
    }

    public boolean isCurrentUser() {
        return currentUser;
    }

    public void setCurrentUser(boolean currentUser) {
        this.currentUser = currentUser;
    }

    public String getMbApply() {
        return mbApply;
    }

    public void setMbApply(String mbApply) {
        this.mbApply = mbApply;
    }

    public String getMbRestore() {
        return mbRestore;
    }

    public void setMbRestore(String mbRestore) {
        this.mbRestore = mbRestore;
    }

    public String getMbLoadSaved() {
        return mbLoadSaved;
    }

    public void setMbLoadSaved(String mbLoadSaved) {
        this.mbLoadSaved = mbLoadSaved;
    }

    public String[] getSubj() {
        return subj;
    }

    public void setSubj(final String[] subj) {
        this.subj = subj;
    }

    public String getChangeByUser() {
        return changeByUser;
    }

    public boolean isRoutesSaved() {
        return routesSaved;
    }

    public void setRoutesSaved(boolean routesSaved) {
        this.routesSaved = routesSaved;
    }

    public boolean isRoutesRestored() {
        return routesRestored;
    }

    public void setRoutesRestored(boolean routesRestored) {
        this.routesRestored = routesRestored;
    }

    public String getRestoreDate() {
        return restoreDate;
    }

    public void setRestoreDate(String restoreDate) {
        this.restoreDate = restoreDate;
    }

    public String getMbLoad() {
        return mbLoad;
    }

    public void setMbLoad(String mbLoad) {
        this.mbLoad = mbLoad;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public boolean isRoutesLoaded() {
        return routesLoaded;
    }

    public void setRoutesLoaded(boolean routesLoaded) {
        this.routesLoaded = routesLoaded;
    }

    private String getUserName(SCAGAppContext appContext) throws SCAGJspException {
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        return user.getName();
    }
}
