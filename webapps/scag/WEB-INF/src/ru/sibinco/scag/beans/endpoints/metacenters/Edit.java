/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.metacenters;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.CancelException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.security.Principal;
import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:59:36
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    public static final String SELECTED_SME_PREFFIX = "selected_sme_";
    protected boolean enabled;
    private String userName = "";
    protected String id = null;
    protected String policy = "";
    private String[] policyTypes = new String[]{"RoundRobin","Random"};
    private String[] policyTitles = new String[]{"RoundRobin","Random"};
    private final String type = Constants.META_TYPE_SMSC;
//    protected String[] selectedSmes = null;
    protected SortedList selectedSmes = new SortedList();
//    protected SortedList deSelectedSmes = new SortedList();
    protected SortedList newSelectedSmes = new SortedList();

    private void init() throws SCAGJspException {
        logger.error( "META:INIT");
        if(isAdd()){
            logger.error( "META:INIT:ADD" );
        }else{
            logger.error( "META:INIT:UPDATE:" + getId() );
//            selectedSmes = readSelectedSmes();
            logger.error( "selectedSmes:" + selectedSmes );
//            selectedSmes = ( (MetaEndpoint)( appContext.getSmppManager().getMetaEndpoints().get( getId() ) ) ).getSmeIds();
        }
    }

    private SortedList readSelectedSmes(){
        SortedList list = new SortedList();
        Map smes = appContext.getSmppManager().getCenters();
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
            if( ((Center)smes.get(key)).getMetaGroup().equals( getId() ) ){
                list.add( key );
            }
        }
        return list;
    }

    private SortedList readSelectedSmes( String id ){
        SortedList list = new SortedList();
        Map smes = appContext.getSmppManager().getCenters();
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
            if( ((Center)smes.get(key)).getMetaGroup().equals(id) ){
                list.add( key );
            }
        }
        return list;
    }

    protected void load(String loadId) throws SCAGJspException {
        logger.error( "META:LOAD:start" );
        final MetaEndpoint meta = (MetaEndpoint) appContext.getSmppManager().getMetaCenters().get(loadId);
        logger.error( "META:LOAD:" + meta.getId() + " | " + meta.getPolicy() + " | " + meta.getType() + " | "
                    + meta.isEnabled() );
        if( meta == null ){
            throw new SCAGJspException(Constants.errors.sme.SME_NOT_FOUND, loadId);
        }
        this.id = meta.getId();
        this.policy = meta.getPolicy();
        this.enabled = meta.isEnabled();
        this.selectedSmes = readSelectedSmes(loadId);
    }

    protected void save() throws SCAGJspException {
        logger.error( "META:SAVE:1" );
        if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
            throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);
//        if( Functions.valdateString( id, Functions.VALIDATION_TYPE_ID )){
//            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID);
//        }
        if( !validateString(id, VALIDATION_TYPE_ID) ){
            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID);
        }
        logger.error( "META:SAVE:2" );
//        final Map svcs = appContext.getSmppManager().getSvcs();
        final Map metas = appContext.getSmppManager().getMetaCenters();
        if( metas.containsKey( id ) && isAdd() ){
            logger.warn( "Such id allready exist!!!");
            throw new SCAGJspException(Constants.errors.sme.METAEP_ALREADY_EXISTS, id);
        }
        MetaEndpoint oldMetaCenter = null;
        if (!isAdd()) {
            oldMetaCenter = (MetaEndpoint) metas.get(getEditId());
            metas.remove( getEditId() );
        }

        logger.error( "META:SAVE:4:getId()=" +getId() );
        MetaEndpoint meta = new MetaEndpoint( getId() );
        logger.error( "META:SAVE:4:getPolicy()=" +getPolicy() );
        meta.setPolicy( getPolicy() );
        meta.setType( type );
        meta.setEnabled( isEnabled() );
        logger.error( "META:SAVE:" + meta.getId() + " | " + meta.getPolicy() + " | " + meta.getType() + " | "
                    + meta.isEnabled() );
        metas.put( getId(), meta );
        if( appContext == null ){
            logger.error( "NULL APPCONTEXT" );
            appContext = getAppContext();
        }
        appContext.getSmppManager().createUpdateMetaEntity( userName, meta, oldMetaCenter, isAdd(), appContext,
                                                            Constants.STORE_TYPE_CENTER);
        updateMetaEPs(meta);
        throw new DoneException();
    }

    public void updateMetaEPs(MetaEndpoint meta){
        Map smes = appContext.getSmppManager().getCenters();
        logger.info( "updateMetaEPs:newSelectedCenters=" + newSelectedSmes);
        for(Iterator iter = newSelectedSmes.iterator(); iter.hasNext();){
            String key = (String)iter.next();
            if(!selectedSmes.contains( key )){
                try {
                    Center center = (Center)smes.get( key );
                    center.setMetaGroup( meta.getId() );
                    logger.info( "updateMetaEPs:updateMetaEndpoint:" + meta.getId() + " | " + key);
                    appContext.getSmppManager().updateMetaEndpoint( userName, meta, key, appContext, true);
                } catch (SCAGJspException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
        logger.info( "updateMetaEPs:selectedCenters=" + selectedSmes);
        selectedSmes = readSelectedSmes( meta.getId() );
        logger.info( "updateMetaEPs:after read selectedCenters=" + selectedSmes);
        for(Iterator iter = selectedSmes.iterator(); iter.hasNext();){
            String key = (String)iter.next();
            if(!newSelectedSmes.contains( key )){
                try {
                    Center center = (Center)smes.get( key );
                    center.setMetaGroup( "" );
                    logger.info( "updateMetaEPs:deleteMetaEndpoint:" + meta.getId() + " | " + key);
                    appContext.getSmppManager().updateMetaEndpoint( userName, meta, key, appContext, false);
                } catch (SCAGJspException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
    }
    public void updateCenters(){
        Map smes = appContext.getSmppManager().getCenters();
        logger.error( "META:updateServices():start:" + smes.keySet() + "|" );
        int count = 0 ;
        for( Iterator iterator = smes.keySet().iterator(); iterator.hasNext(); ){
            logger.error( "ITER:" + ++count );
            Center center = null;
            String key = (String)iterator.next();
            if( newSelectedSmes.contains(key) && !readSelectedSmes().contains(key) ){
                logger.error( "Center id in selected=" + key );
                center = (Center)smes.get( key );
                center.setMetaGroup( getId() );
            } else if( readSelectedSmes().contains(key) && !newSelectedSmes.contains(key) ){
                logger.error( "Center id in deselected=" + key );
                center = (Center)smes.get( key );
                center.setMetaGroup( "" );
            }
            if( center != null ){
                try {
                    logger.error( "Update svc with id='" + center.getId() + "'" );
                    appContext.getSmppManager().createUpdateCenter( userName,
                            false, center.isEnabled(), center, appContext, center);
                } catch (SCAGJspException e) {
                    logger.error( "Exception while updateSmes:Svcs" );
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
//        try {
//            appContext.getSmppManager().store();
//        } catch (SibincoException e) {
//            logger.error( "Exception while update SME for MetaEndpoint" );
//            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
//        }
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

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
        loginedPrincipal = request.getUserPrincipal();
        userName = loginedPrincipal.getName();
        logger.error( "META:INIT:USERNAME" + userName);
        if (getMbCancel() != null){
            throw new CancelException();
        }
        logger.error( "PROCESS:1");
        appContext = getAppContext();
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }
//        super.process(request, response);
        if (getId() == null && !isAdd())
            load(getEditId());

        if (getMbSave() != null){
            logger.debug("meta:Edit:process():request.getParameterMap().entrySet()=" + request.getParameterMap().entrySet() );
            logger.debug("meta:Edit:process():request.getParameterMap().keySet()=" + request.getParameterMap().keySet() );
            for( Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext();){
                String key = (String)iter.next();
                String smeName = null;
//                logger.error( "META:EDIT:PROCESS:KEY=" + key + "|'" + request.getParameterMap().get(key) + "'");
                if( key.startsWith( SELECTED_SME_PREFFIX) ){
//                    logger.error( "META:EDIT:PROCESS:SELECTED_SME:KEY=" + key + "|'" + request.getParameterMap().get(key) + "'");
                    smeName = key.substring( SELECTED_SME_PREFFIX.length() );
                    newSelectedSmes.add( smeName );
                }
//                if( key.startsWith( DESELECTED_SME_PREFFIX) ){
//                    logger.error( "META:EDIT:PROCESS:DESELECTED_SME:KEY=" + key + "|'" + request.getParameterMap().get(key) + "'");
//                    smeName = key.substring( SELECTED_SME_PREFFIX.length() );
//                    deSelectedSmes.add( smeName );
//                }
            }
            save();
        }
        this.init();
        logger.error( "PROCESS:2");
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getId() {
        if(id != null)id.trim();
        return id;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public boolean getEnabled(){
        return enabled;
    }

    public List getAvailableSmes() {
        SortedList list = new SortedList();

        Map smes = appContext.getSmppManager().getCenters();
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
            logger.error( "KEY=" + key );
            Center  center = (Center)smes.get(key);
            if( center.getMetaGroup().equals("") ){
                logger.error( "ADD TO AVAILABLE '" + center.getId() + "'");
                list.add( key );
            }
        }
//        list.addAll(appContext.getSmppManager().getCenters().keySet());
//        list.addAll(appContext.getSmppManager().getSvcs().keySet());
        logger.error( "Available smes=" + list );
        return list;
    }

    public List getHardSmes(){
        SortedList list = new SortedList();
        list.add( "smschard1" );
        list.add( "smschard2" );
        list.add( "smschard3" );
        return list;
    }

    public SortedList getSelectedSmes(){
//        logger.error( "getSelectedSmes:" + selectedSmes );
        return selectedSmes;
    }

    public String getPolicy() {
        return policy;
    }

    public void setPolicy(String policy) {
        this.policy = policy;
    }

    public String[] getPolicyTypes() {
        return policyTypes;
    }

    public void setPolicyTypes(String[] policyTypes) {
        this.policyTypes = policyTypes;
    }

    public String[] getPolicyTitles() {
        return policyTitles;
    }

    public void setPolicyTitles(String[] policyTitles) {
        this.policyTitles = policyTitles;
    }
}