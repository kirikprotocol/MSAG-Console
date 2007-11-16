/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.metaservices;

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
    private String[] policyTypes = new String[]{"roundrobin","random"};
    private String[] policyTitles = new String[]{"RoundRobin","Random"};
    private final String type = Constants.META_TYPE_SERVICE;
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
        Map smes = appContext.getSmppManager().getSvcs();
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
            if( ((Svc)smes.get(key)).getMetaGroup().equals( getId() ) ){
                list.add( key );
            }
        }
        return list;
    }

    protected void load(String loadId) throws SCAGJspException {
        final MetaEndpoint meta = (MetaEndpoint) appContext.getSmppManager().getMetaServices().get(loadId);
        logger.error( "METASERVICE:LOAD:" + meta.getId() + " | " + meta.getPolicy() + " | " + meta.getType() + " | "
                    + meta.isEnabled() );
        if( meta == null ){
            throw new SCAGJspException(Constants.errors.sme.SME_NOT_FOUND, loadId);
        }
        this.id = meta.getId();
        this.policy = meta.getPolicy();
        this.enabled = meta.isEnabled();
        this.selectedSmes = readSelectedSmes(loadId);
        logger.info( "METASERVICE:selectedSmes=" + this.selectedSmes );
    }

    private SortedList readSelectedSmes( String id ){
        SortedList list = new SortedList();
        Map smes = appContext.getSmppManager().getSvcs();
        logger.error( "METASERVICE:readSelectedSmes:smes.keySet:" + smes.keySet() );
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
//            logger.error( "READ KEY=" + key + " | metaGroup=" + ((Svc)smes.get(key)).getMetaGroup());
            if( ((Svc)smes.get(key)).getMetaGroup().equals(id) ){
//                logger.error( "READ LOAD KEY=" + key );
                list.add( key );
            }
        }
        logger.info( "METASERVICE:Load selected:" + list );
        return list;
    }

    protected void save() throws SCAGJspException {
        if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
            throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);
//        if( Functions.valdateString( id, Functions.VALIDATION_TYPE_ID )){
//            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID);
//        }
        if( !validateString(id, VALIDATION_TYPE_ID) ){
            throw new SCAGJspException(Constants.errors.sme.COULDNT_SAVE_NOT_VALID_ID);
        }
        final Map metas = appContext.getSmppManager().getMetaServices();
        if( metas.containsKey( id ) && isAdd() ){
            logger.warn( "METASERVICE:Such id allready exist!!!");
            throw new SCAGJspException(Constants.errors.sme.METAEP_ALREADY_EXISTS, id);
        }
        MetaEndpoint oldMetaSvc = null;
        if (!isAdd()) {
            oldMetaSvc = (MetaEndpoint) metas.get(getEditId());
            metas.remove( getEditId() );
        }

        MetaEndpoint meta = new MetaEndpoint( getId() );
        meta.setPolicy( getPolicy() );
        meta.setType( type );
        meta.setEnabled( isEnabled() );
        logger.error( "METASERVICE:SAVE:" + meta.getId() + " | " + meta.getPolicy() + " | " + meta.getType() + " | " + meta.isEnabled() );
        metas.put( getId(), meta );
        if( appContext == null ){
            logger.error( "METASERVICE:NULL APPCONTEXT" );
            appContext = getAppContext();
        }
//        logger.error( "super.Principal:" + super.getLoginedPrincipal().getName() );
//        Principal userPrincipal = super.getLoginedPrincipal();
//        if (userPrincipal == null)
//            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
//        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());

//        appContext.getSmppManager().createUpdateMetaEndpoints(super.getLoginedPrincipal().getName(), meta, isAdd(), appContext);
        appContext.getSmppManager().createUpdateMetaEntity( userName, meta, oldMetaSvc, isAdd(), appContext,
                                                            Constants.STORE_TYPE_SVC );
        updateMetaEPs(meta);
        throw new DoneException();
    }

    public void updateMetaEPs(MetaEndpoint meta){
        Map smes = appContext.getSmppManager().getSvcs();
        logger.info( "METASERVICE:updateMetaEPs:newSelectedCenters=" + newSelectedSmes + " selectedSmes=" + selectedSmes );
        for(Iterator iter = newSelectedSmes.iterator(); iter.hasNext();){
            String key = (String)iter.next();
            logger.info( "METASERVICE:updateMetaEPs:add:service:" + key);
            if(!selectedSmes.contains( key )){
                try {
                    Svc svc = (Svc)smes.get( key );
                    svc.setMetaGroup( meta.getId() );
                    logger.info( "METASERVICE:updateMetaEPs:addMetaEndpoint:" + meta.getId() + " | " + key);
                    appContext.getSmppManager().updateMetaEndpoint( userName, meta, key, appContext, true);
                } catch (SCAGJspException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
        for(Iterator iter = selectedSmes.iterator(); iter.hasNext();){
            String key = (String)iter.next();
            logger.info( "METASERVICE:updateMetaEPs:delete:service:" + key);
            if(!newSelectedSmes.contains( key )){
                try {
                    Svc svc = (Svc)smes.get( key );
                    svc.setMetaGroup( "" );
                    logger.info( "METASERVICE:updateMetaEPs:deleteMetaEndpoint:" + meta.getId() + " | " + key);
                    appContext.getSmppManager().updateMetaEndpoint( userName, meta, key, appContext, false);
                } catch (SCAGJspException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
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
        logger.info( "METASERVICE:process:USERNAME " + userName);
                if (getMbCancel() != null){
            throw new CancelException();
        }
        appContext = getAppContext();
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }
//        super.process(request, response);
        if (getId() == null && !isAdd())
            load(getEditId());

        if (getMbSave() != null){
            logger.debug("METASERVICE:process():request.getParameterMap().entrySet()=" + request.getParameterMap().entrySet() );
            logger.debug("METASERVICE:process():request.getParameterMap().keySet()=" + request.getParameterMap().keySet() );
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
            selectedSmes = readSelectedSmes(getId());
            save();
        }
        logger.info( "METASERVICE:PROCESS:end");
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

        Map smes = appContext.getSmppManager().getSvcs();
        for( Iterator iter = smes.keySet().iterator(); iter.hasNext();){
            String key = (String)iter.next();
//            logger.error( "KEY=" + key );
            Svc  svc = (Svc)smes.get(key);
            if( svc.getMetaGroup().equals("") ){
//                logger.error( "METASERVICE:ADD TO AVAILABLE '" + svc.getId() + "'");
                list.add( key );
            }
        }
//        list.addAll(appContext.getSmppManager().getCenters().keySet());
//        list.addAll(appContext.getSmppManager().getSvcs().keySet());
        logger.debug( "METASERVICE:Available smes=" + list );
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