package ru.sibinco.scag.beans.routing.routes;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.text.DateFormat;
import java.util.*;


/**
 * Created by igork Date: 19.04.2004 Time: 17:39:13
 */
public class Index extends TabledBeanImpl {

    private String mbApply;
    private String mbRestore;
    private String mbLoadSaved;
    private String mbLoad;
    private String mbSave;

    private String mbApplyHttp;
    private String mbLoadSavedHttp;
    private String mbLoadHttp;
    private String mbSaveHttp;
    private String mbRestoreHttp;

    private String[] subj;
    private boolean routesChanged;
    private String  changeByUser;
    private boolean currentUser;
    private boolean routesSaved = false;
    private boolean routesRestored = false;
    private String  restoreDate = null;
    private boolean routesLoaded = false;

    private boolean httpRoutesSaved = false;
    private boolean httpRoutesChanged = false;
    private boolean httpRoutesRestored = false;
    private boolean httpRoutesLoaded = false;

    protected long transportId = Transport.SMPP_TRANSPORT_ID;


    protected Collection getDataSource() {
        if (transportId == Transport.SMPP_TRANSPORT_ID) {
            return appContext.getScagRoutingManager().getStatMessages();
        } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
            return appContext.getHttpRoutingManager().getStatMessages();
        } else {
            return new HashMap().values();
        }
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        logger.debug("routes.Index.process():start");
        sort = "time";
        super.process(request, response);
        logger.debug("routes.Index.process()\n"
                + "mbRestoreHttp:'" + mbRestoreHttp +"'\n"
                + "mbRestore:'" + mbRestore+"'\n"
                + "mbApplyHttp:'" + mbApplyHttp +"'\n"
                + "mbApply:'" + mbApply +"'\n"
                + "mbLoadSaved:'" + mbLoadSaved +"'\n"
                + "mbLoadSavedHttp:'" + mbLoadSavedHttp +"'\n"
                + "mbLoad:'" + mbLoad +"'\n"
                + "mbLoadHttp:'" + mbLoadHttp +"'\n"
                + "mbSave:'" + mbSave +"'\n"
                + "mbSaveHttp:'" + mbSaveHttp +"'\n"
        );
        if (mbRestoreHttp != null){
            restoreHttpRoutes();
        }
        if (mbRestore != null){
            restoreRoutes();
        }
        else if (mbApplyHttp != null){
            applyHttpRoutes();
        }
        else if (mbApply != null){
            logger.debug( "routes/Index.process() mbApply" );
            applySmppRoutes();
        }
        else if (mbLoadSaved != null){
            restoreRoutes();
        }
        else if (mbLoadSavedHttp != null){
            restoreHttpRoutes();
        }
        else if (mbLoad != null){
            loadRoutes();
        }
        else if (mbLoadHttp != null){
            loadHttpRoutes();
        }
        else if (mbSave != null){
            saveRoutes();
        }
        else if (mbSaveHttp != null){
            saveHttpRoutes();
        }
        this.init();
        //refresh changes bean values (it's bug from class TabledBeanImpl)
        logger.debug("routes/Index.process() end");
        super.process(request, response);
        if( tabledItems.isEmpty() ){
            logger.debug("DEBUG:process():tabledItems.isEmpty(): set routesRestored to 'true' " +
                    "routesRestored was '" + routesRestored + "'");
            routesRestored = true;
        }
    }

    private void init() throws SCAGJspException {
        SCAGAppContext appContext = getAppContext();

        final Locale locale = new Locale(Functions.getLocaleEncoding());
        Calendar restoreCalendar = new GregorianCalendar(locale);
        DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM, locale);

        Date restoreFileDate = null;
        try {
            if (transportId == Transport.SMPP_TRANSPORT_ID) {
                restoreFileDate = appContext.getScagRoutingManager().getRestoreFileDate();
            } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
                restoreFileDate = appContext.getHttpRoutingManager().getRestoreFileDate();
            }
            if (restoreFileDate != null) {
                restoreCalendar.setTime(restoreFileDate);
                restoreDate = dateFormat.format(restoreFileDate);
            }
        } catch (SibincoException e) {
            e.printStackTrace();
        }

        /********************** SMPP ROUTES *********************************/
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

        /*********************** HTTP ROUTES **********************************/
        httpRoutesRestored = appContext.getHttpRoutingManager().isRoutesRestored();
        httpRoutesSaved = appContext.getHttpRoutingManager().isRoutesSaved();
        if (appContext.getHttpRoutingManager().hasSavedConfiguration()) {
            httpRoutesLoaded = appContext.getHttpRoutingManager().isRoutesLoaded();
        }
        httpRoutesChanged = appContext.getHttpRoutingManager().isRoutesChanged();
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
     * This private method will be restory routes from smpp_routes_.xml file
     * saved user's earlier
     *
     * @throws SCAGJspException - an throws exception if it will be occurred
     */
    private void restoreRoutes() throws SCAGJspException {
        try {
            appContext.getScagRoutingManager().restore();
            appContext.getScagRoutingManager().setRoutesChanged(true);//???
            appContext.getScagRoutingManager().setRoutesRestored(true);
            appContext.getScagRoutingManager().setRoutesSaved(false);
            appContext.getScagRoutingManager().setRoutesLoaded(false);
        } catch (SibincoException e) {
            logger.debug("ERROR:Couldn't restore routes", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_RESTORE_ROUTES, e);
        }
    }

    /**
     * This private method will be restory routes from http_routes_.xml file
     * saved user's earlier
     *
     * @throws SCAGJspException - an throws exception if it will be occurred
     */
    private void restoreHttpRoutes() throws SCAGJspException {
        try {
            appContext.getHttpRoutingManager().restore();
            appContext.getHttpRoutingManager().setRoutesChanged(true);
            appContext.getHttpRoutingManager().setRoutesRestored(true);
            appContext.getHttpRoutingManager().setRoutesSaved(false);
            appContext.getHttpRoutingManager().setRoutesLoaded(false);

        } catch (SibincoException e) {
            logger.debug("Couldn't restore http routes", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_RESTORE_HTTP_ROUTES, e);
        }
    }

    /**
     * This private method will be applay smpp routes to smpp_routes.xml file
     *
     * @throws SCAGJspException - an throws exception if it will be occurred
     */
    private void applySmppRoutes() throws SCAGJspException {
        logger.debug( "routes/Index.applySmppRoutes() start" );
        appContext.getScagRoutingManager().apply(appContext);
        appContext.getScagRoutingManager().setChangedByUser("");
        appContext.getScagRoutingManager().setRoutesChanged(false);
        init();
    }

    /**
     * This private method will be applay http routes to http_routes.xml file
     *
     * @throws SCAGJspException - an throws exception if it will be occurred
     */
    private void applyHttpRoutes() throws SCAGJspException {
        appContext.getHttpRoutingManager().apply(appContext);
        appContext.getHttpRoutingManager().setRoutesChanged(false);
        init();
    }

    /**
     * This method provides to load smpp routes applied configuration
     *
     * @throws SCAGJspException - an throws exception if it will
     *                          be occurred@throws SCAGJspException
     */
    protected void loadRoutes() throws SCAGJspException {

        try {
            appContext.getScagRoutingManager().load();
        } catch (SibincoException exc) {
            logger.debug("Couldn't load smpp routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_LOAD_ROUTES, exc);
        }
        appContext.getScagRoutingManager().setRoutesChanged(false);
        appContext.getScagRoutingManager().setRoutesRestored(false);
        appContext.getScagRoutingManager().setRoutesLoaded(true);
        appContext.getScagRoutingManager().setRoutesSaved(false);
    }

    /**
     * This method provides to load http routes applied configuration
     *
     * @throws SCAGJspException - an throws exception if it will
     *                          be occurred@throws SCAGJspException
     */
    protected void loadHttpRoutes() throws SCAGJspException {

        try {
            appContext.getHttpRoutingManager().load();
        } catch (SibincoException exc) {
            logger.debug("Couldn't load http routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_LOAD_HTTP_ROUTES, exc);
        }
        appContext.getHttpRoutingManager().setRoutesChanged(false);
        appContext.getHttpRoutingManager().setRoutesRestored(false);
        appContext.getHttpRoutingManager().setRoutesLoaded(true);
        appContext.getHttpRoutingManager().setRoutesSaved(false);
    }

    /**
     * This method providers to saves temporal smpp routes  configuration
     *
     * @throws SCAGJspException - an throws exception if it will
     *                          be occurred@throws SCAGJspException
     */
    protected void saveRoutes() throws SCAGJspException {
        try {
            appContext.getScagRoutingManager().save();
        } catch (SibincoException exc) {
            logger.debug("Couldn't save smpp routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_SAVE_ROUTES, exc);
        }
        appContext.getScagRoutingManager().setRoutesSaved(true);
//        logic bug
//        appContext.getScagRoutingManager().setRoutesRestored(true);
    }

    /**
     * This method providers to saves temporal http routes  configuration
     *
     * @throws SCAGJspException - an throws exception if it will
     *                          be occurred@throws SCAGJspException
     */
    protected void saveHttpRoutes() throws SCAGJspException {
        try {
            appContext.getHttpRoutingManager().save();
        } catch (SibincoException exc) {
            logger.debug("Couldn't save http routes", exc);
            throw new SCAGJspException(Constants.errors.routing.routes.COULDNT_SAVE_HTTP_ROUTES, exc);
        }
        appContext.getHttpRoutingManager().setRoutesSaved(true);
//        logic bug
//        appContext.getHttpRoutingManager().setRoutesRestored(true);
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

    public String getMbApplyHttp() {
        return mbApplyHttp;
    }

    public void setMbApplyHttp(String mbApplyHttp) {
        this.mbApplyHttp = mbApplyHttp;
    }

    public String getMbLoadSavedHttp() {
        return mbLoadSavedHttp;
    }

    public void setMbLoadSavedHttp(String mbLoadSavedHttp) {
        this.mbLoadSavedHttp = mbLoadSavedHttp;
    }

    public String getMbLoadHttp() {
        return mbLoadHttp;
    }

    public void setMbLoadHttp(String mbLoadHttp) {
        this.mbLoadHttp = mbLoadHttp;
    }

    public String getMbSaveHttp() {
        return mbSaveHttp;
    }

    public void setMbSaveHttp(String mbSaveHttp) {
        this.mbSaveHttp = mbSaveHttp;
    }

    public String getMbRestoreHttp() {
        return mbRestoreHttp;
    }

    public void setMbRestoreHttp(String mbRestoreHttp) {
        this.mbRestoreHttp = mbRestoreHttp;
    }

    public boolean isHttpRoutesChanged() {
        return httpRoutesChanged;
    }

    public void setHttpRoutesChanged(boolean httpRoutesChanged) {
        this.httpRoutesChanged = httpRoutesChanged;
    }

    public boolean isHttpRoutesRestored() {
        return httpRoutesRestored;
    }

    public void setHttpRoutesRestored(boolean httpRoutesRestored) {
        this.httpRoutesRestored = httpRoutesRestored;
    }

    public boolean isHttpRoutesLoaded() {
        return httpRoutesLoaded;
    }

    public void setHttpRoutesLoaded(boolean httpRoutesLoaded) {
        this.httpRoutesLoaded = httpRoutesLoaded;
    }

    public boolean isHttpRoutesSaved() {
        return httpRoutesSaved;
    }

    public void setHttpRoutesSaved(boolean httpRoutesSaved) {
        this.httpRoutesSaved = httpRoutesSaved;
    }
}

