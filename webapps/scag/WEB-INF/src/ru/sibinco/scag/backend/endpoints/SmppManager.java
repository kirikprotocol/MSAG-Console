/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.centers.CenterStatus;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.svc.SvcStatus;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * The <code>SmppManager</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 12:39:13
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppManager extends Manager {

    private static final String PARAM_NAME_LAST_UID_ID = "last used uid";

    private Logger logger = Logger.getLogger(this.getClass());

    private final Map svcs = Collections.synchronizedMap(new HashMap());
    private final Map centers = Collections.synchronizedMap(new HashMap());
//    private final Map metaEndpoints = Collections.synchronizedMap(new HashMap());
    private final Map metaCenters = Collections.synchronizedMap(new HashMap());
    private final Map metaServices = Collections.synchronizedMap(new HashMap());
    private final String configFilename;
    private final String metaConfigFilename = "";
    private final ProviderManager providerManager;

    private int lastUsedId = -1;


    public SmppManager(String configFilename, ProviderManager providerManager) {
        logger.debug( "SmppManager(String string, ProviderManager providerManager)ConfigFilename: " + configFilename );
        this.configFilename = configFilename;
        this.providerManager = providerManager;
    }

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        svcs.clear();
        centers.clear();
        metaServices.clear();
        metaCenters.clear();
//        metaEndpoints.put( "MetaName1", new MetaEndpoint("MetaName1") );

//        metaServices.put( "MetaService1", new MetaEndpoint("MetaService1" ) );
//        metaServices.put( "MetaService2", new MetaEndpoint("MetaService2" ) );
//        metaServices.put( "MetaService3", new MetaEndpoint("MetaService3" ) );
//
//        metaCenters.put( "MetaCenter1", new MetaEndpoint("MetaCenter1") );
//        metaCenters.put( "MetaCenter2", new MetaEndpoint("MetaCenter2") );
//        metaCenters.put( "MetaCenter3", new MetaEndpoint("MetaCenter3") );
//
        if (configFilename != null) {
            final Document document = Utils.parse(configFilename);
            final NodeList paramConfs = document.getDocumentElement().getElementsByTagName("param_conf");

            for (int i = 0; i < paramConfs.getLength(); i++) {
                final Element paramElem = (Element) paramConfs.item(i);
                final String name = paramElem.getAttribute("name");
                try {
                    if (PARAM_NAME_LAST_UID_ID.equals(name)) {
                        lastUsedId = Integer.decode(Utils.getNodeText(paramConfs.item(i))).intValue();
                    }
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                    logger.error("Int parameter \"" + name + "\" misformatted: " + lastUsedId + ", skipped", e);
                }
            }

            final NodeList smerecords = document.getDocumentElement().getElementsByTagName(Constants.SME_RECORD_TAG);
            for (int i = 0; i < smerecords.getLength(); i++) {
                final Element svcRecords = (Element) smerecords.item(i);
                final Svc service = createSvc(svcRecords);
                svcs.put(service.getId(), service);
            }
            final NodeList centerRecords = document.getDocumentElement().getElementsByTagName(Constants.SMSC_RECORD_TAG);
            for (int i = 0; i < centerRecords.getLength(); i++) {
                final Element centersRecords = (Element) centerRecords.item(i);
                final Center center = createCenter(centersRecords);
                centers.put(center.getId(), center);
            }            
            final NodeList metasmeRecords = document.getDocumentElement().getElementsByTagName(Constants.META_SME_RECORD_TAG);
            for (int i = 0; i < metasmeRecords.getLength(); i++) {
                final Element metaRecord = (Element) metasmeRecords.item(i);
                final MetaEndpoint meta = createMetaEndpoint(metaRecord);
                metaServices.put(meta.getId(), meta);
            }
            final NodeList metasmscRecords = document.getDocumentElement().getElementsByTagName(Constants.META_SMSC_RECORD_TAG);
            for (int i = 0; i < metasmscRecords.getLength(); i++) {
                final Element metaRecord = (Element) metasmscRecords.item(i);
                final MetaEndpoint meta = createMetaEndpoint(metaRecord);
                metaCenters.put(meta.getId(), meta);
            }
        }
        if( metaConfigFilename != null ){
            final Document document = Utils.parse(configFilename);
            final NodeList paramConfs = document.getDocumentElement().getElementsByTagName("param_conf");

        }
    }

    private Svc createSvc(Element svcRecords) {
        return new Svc(svcRecords, providerManager);
    }

    protected Center createCenter(Element centersRecords) {
        return new Center(centersRecords, providerManager);
    }

    protected MetaEndpoint createMetaEndpoint(Element metaRecord) {
        return new MetaEndpoint(metaRecord, providerManager);
    }


    public synchronized void deleteCenters(final String user, final Set checkedSet, final SCAGAppContext appContext) throws SCAGJspException {
        final Map centers = getCenters();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String centerId = (String) iterator.next();
            Center center = (Center) centers.get(centerId);
            centers.remove(centerId);
            try {
//                if (center.isEnabled()) {
                    appContext.getScag().invokeCommand("deleteCenter",center,appContext,this,configFilename);
//                } else {
//                    logger.error( "Deleted from file only center with id:" +centerId );
//                    store();
//                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete Smsc \"" + centerId + '"', e);
                    centers.put(centerId,center);
                    try {
                        store();
                    } catch (SibincoException e1) {
                        e1.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                    }
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, centerId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", "Deleted center(s): "
                + checkedSet.toString() + "."));
    }

    public synchronized void deleteMetaEndpoints(final String user, final Set checkedSet, final SCAGAppContext appContext) throws SCAGJspException {
//        final Map metas = getMetaEndpoints();
//        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
//            final String metaId = (String) iterator.next();
//            MetaEndpoint meta = (MetaEndpoint) metaEndpoints.get(metaId);
//            metaEndpoints.remove(metaId);
//            try {
//                if (meta != null) {
//                    appContext.getScag().invokeCommand("deleteMetaEndpoint",meta,appContext,this,configFilename);
//                } else {
//                    logger.error( "Deleted from file only center with id:" + metaId );
//                    storeMeta();
//                }
//            } catch (SibincoException e) {
//                if (!(e instanceof StatusDisconnectedException)) {
//                    logger.error("Couldn't delete Smsc \"" + metaId + '"', e);
//                    metaEndpoints.put(metaId, meta);
//                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, metaId, e);
//                }
//            }
//        }
//        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", "Deleted center(s): "
//                + checkedSet.toString() + "."));
    }

    public synchronized void deleteMetaServices(final String user, final Set checkedSet, final SCAGAppContext appContext) throws SCAGJspException {
        final Map metas = getMetaServices();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String metaId = (String) iterator.next();
            MetaEndpoint meta = (MetaEndpoint) metas.get(metaId);
            Map map = getServicesByMetaGroupId(metaId);
            for(Iterator iter = map.keySet().iterator(); iter.hasNext();){
                Svc  svc = (Svc)getSvcs().get(iter.next());
                if( svc != null ){
                    svc.setMetaGroup("");
                    logger.error( "Clear SVC, id:" + svc.getId() );
                    try {
                        appContext.getScag().invokeCommand("removeMetaEndpoint", svc, appContext, this, configFilename);
                    } catch (SibincoException e) {
                        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                    }
                }
            }
            logger.error( "DELETE METASERVICE, id:" + metaId );
            metaServices.remove(metaId);
            try {
                if (meta != null) {
                    appContext.getScag().invokeCommand("deleteMetaEntity", meta, appContext, this, configFilename);
                } else {
                    logger.error( "Deleted from file only center with id:" + metaId );
                    store();
                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete MetaService \"" + metaId + '"', e);
                    metaServices.put(metaId, meta);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, metaId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service", "Deleted service(s): "
                + checkedSet.toString() + "."));
    }

    public synchronized void deleteMetaCenters(final String user, final Set checkedSet, final SCAGAppContext appContext) throws SCAGJspException {
        final Map metas = getMetaCenters();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String metaId = (String) iterator.next();
            MetaEndpoint meta = (MetaEndpoint) metas.get(metaId);
            Map map = getCentersByMetaGroupId(metaId);
            for(Iterator iter = map.keySet().iterator(); iter.hasNext();){
//                Center center = (Center)getCenters().get(iter.next());
                Center center = (Center) map.get( iter.next() );
                if( center != null ){
                    center.setMetaGroup("");
                    logger.error( "Clear CENTER, id:" + center.getId() );
                    try {
                        appContext.getScag().invokeCommand("removeMetaEndpoint", center, appContext, this, configFilename);
                    } catch (SibincoException e) {
                        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                    }
                }
            }
            logger.error( "DELETE METACENTER, id:" + metaId );
            metaCenters.remove(metaId);
            try {
                if (meta != null) {
                    appContext.getScag().invokeCommand("deleteMetaEntity", meta, appContext, this, configFilename);
                } else {
                    logger.error( "Deleted from file only center with id:" + metaId );
                    storeMeta();
                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete MetaCenter \"" + metaId + '"', e);
                    metaCenters.put(metaId, meta);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, metaId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", "Deleted center(s): "
                + checkedSet.toString() + "."));
    }

    public void storeMeta(){
        logger.error( "storeMeta" );
    }

    public synchronized void deleteServicePoints(final String user, final Set checkedSet, final SCAGAppContext appContext) throws SCAGJspException {
        final Map svcs = getSvcs();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String svcId = (String) iterator.next();
            Svc svc = (Svc) svcs.get(svcId);
            svcs.remove(svcId);
            try {
//                if (svc.isEnabled()) {
                    appContext.getScag().invokeCommand("deleteSvc",svcId,appContext,this,configFilename);
//                } else {
//                    logger.error("Delete SP in FILE ONLY, id=" + svcId );
//                    store();
//                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete sme \"" + svcId + '"', e);
                    svcs.put(svcId,svc);
                    try {
                        store();
                    } catch (SibincoException e1) {
                        e1.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                    }
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, svcId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", "Deleted service point(s): "
                + checkedSet.toString() + "."));
    }

    public synchronized void disconnectServices(final String user, final String[] serviceIds, final SCAGAppContext appContext) throws SCAGJspException {
      for (int i=0;i<serviceIds.length;i++) {
        try {
            appContext.getScag().disconnectService(serviceIds[i]);
        } catch (SibincoException e) {
          if (!(e instanceof StatusDisconnectedException)) {
            logger.error("Couldn't disconnect services \"" + serviceIds + '"', e);
            throw new SCAGJspException(Constants.errors.sme.COULDNT_DISCONNECT,serviceIds[i],e);
          } else
              throw new SCAGJspException(Constants.errors.sme.COULDNT_DISCONNECT0,e);
        }
      }
      StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", "Disconnected service point(s): " + Arrays.asList(serviceIds) + "."));
    }

    public synchronized void createUpdateCenter(String user, boolean isAdd, boolean isEnabled, Center center,
                                                SCAGAppContext appContext, Center oldCenter) throws SCAGJspException {
        String messageText = "";
//        if( oldCenter == null) oldCenter = center;
        try {
            if (isAdd) {
                messageText = "Added new center: ";
                int uid = getLastUsedId();
                center.setUid(++uid);
                setLastUsedId(center.getUid());
                appContext.getScag().invokeCommand("addCenter",center,appContext,this,configFilename);
            } else {
                messageText = "Changed center: ";
                logger.error("UPDATE CENTER id=" + center.getId() );
                appContext.getScag().invokeCommand("updateCenter",center,appContext,this,configFilename);
            }
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                centers.remove(center.getId());
                if (!isAdd) centers.put(oldCenter.getId(),oldCenter);
                logger.error("Couldn't apply Centers " + center.getId() + " ", e);
                try {
                    store();
                } catch (SibincoException e1) {
                    logger.error("Couldn't restore Centers " + center.getId() + " ", e1);
                }
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, center.getId(), e);
            }
        }
        finally {
            oldCenter = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", messageText + center.getId()));
    }

//    public synchronized void updateCenter(String user, boolean isAdd, boolean isEnabled, Center center,
//                                                SCAGAppContext appContext, Center oldCenter) throws SCAGJspException {
//        String messageText = "";
//        try {
//                messageText = "Changed center: ";
//                if (oldCenter.isEnabled() == center.isEnabled()) {
//                    if (isEnabled)
//                        appContext.getScag().invokeCommand("updateCenter",center,appContext,this,configFilename);
//                } else {
//                    if (center.isEnabled()) {
//                        appContext.getScag().invokeCommand("addCenter",center,appContext,this,configFilename);
//                    } else {
//                        appContext.getScag().invokeCommand("deleteCenter",center,appContext,this,configFilename);
//                    }
//                }
//        } catch (SibincoException e) {
//            if (!(e instanceof StatusDisconnectedException)) {
//                centers.remove(center.getId());
//                if (!isAdd) centers.put(oldCenter.getId(),oldCenter);
//                logger.error("Couldn't applay Centers " + center.getId() + " ", e);
//                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, center.getId(), e);
//            }
//        }
//        finally {
//            oldCenter = null;
//        }
//        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", messageText + center.getId()));
//    }
    public synchronized void createUpdateServicePoint(String user, Svc svc, boolean isAdd,
                                                      boolean isEnabled, SCAGAppContext appContext,
                                                      Svc oldSvc) throws SCAGJspException {
        logger.error("SmppManager.createUpdateServicePoint() start");
        String messageText = "";
        try {
            if (isAdd) {
                logger.error("SmppManager.createUpdateServicePoint() isAdd");
                messageText = "Added new service point: ";
                appContext.getScag().invokeCommand("addSvc", svc, appContext, this, configFilename);
            } else {
                logger.error("SmppManager.createUpdateServicePoint() !isAdd");
                messageText = "Changed service point: ";
                appContext.getScag().invokeCommand("updateSvc", svc, appContext, this, configFilename);
            }
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                svcs.remove(svc.getId());
                if (!isAdd) svcs.put(oldSvc.getId(),oldSvc);
                try {
                    store();
                } catch (SibincoException e1) {
                    logger.error("Couldn't restore service points " + svc.getId() + " ", e);
                }
                logger.error("Couldn't apply service point " + svc.getId() + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, svc.getId(), e);
            }
        } finally {
            oldSvc = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", messageText + svc.getId()));
    }

    public synchronized void createUpdateServicePoint(
            String user, Svc svc, boolean isAdd, SCAGAppContext appContext, Svc oldSvc
            )throws SCAGJspException {
        String messageText = "";
        try {
            if (isAdd) {
                messageText = "Added new service point: ";
                appContext.getScag().invokeCommand("addSvc", svc, appContext, this, configFilename);
            } else {
                messageText = "Changed service point: ";
                appContext.getScag().invokeCommand("updateSvc", svc, appContext, this, configFilename);
            }
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                svcs.remove(svc.getId());
                if (!isAdd) svcs.put(oldSvc.getId(),oldSvc);
                try {
                    store();
                } catch (SibincoException e1) {
                    logger.error("Couldn't restore service points " + svc.getId() + " ", e);
                }
                logger.error("Couldn't apply service point " + svc.getId() + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, svc.getId(), e);
            }
        } finally {
            oldSvc = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", messageText + svc.getId()));
    }


//    public synchronized void createUpdateServicePoint(
//            String user, Svc svc, boolean isAdd, boolean isEnabled, SCAGAppContext appContext,
//            Svc oldSvc, boolean updateMeta
//            ) throws SCAGJspException {
//        String messageText = "";
//        try {
//            if (isAdd) {
//                messageText = "Added new service point: ";
//                if (svc.isEnabled()) {
//                    appContext.getScag().invokeCommand("addSvc", svc, appContext, this, configFilename);
//                }else{
//                    logger.error("SAVE SP TO FILE ONLY, id=" + svc.getId() );
//                    store();
//                }
//            } else {
//                messageText = "Changed service point: ";
//                if ((oldSvc.isEnabled() == svc.isEnabled())) {
//                    if (isEnabled){
//                        if(!updateMeta){
//                            appContext.getScag().invokeCommand("updateSvcInfo", svc, appContext, this, configFilename);
//                        } else{
//                            logger.error( "SMPPMANAGER UPDATE METAGROUP");
//                            appContext.getScag().invokeCommand("updateMetaGroup", svc, appContext, this, configFilename);
//                        }
//                    }else{
//                        store();
//                    }
//                } else {
//                    if (svc.isEnabled()) {
//                        appContext.getScag().invokeCommand("addSvc", svc, appContext, this, configFilename);
//                    } else {
//                        appContext.getScag().invokeCommand("deleteSvc", svc.getId(), appContext, this, configFilename);
//                    }
//                }
//            }
//        } catch (SibincoException e) {
//            if (!(e instanceof StatusDisconnectedException)) {
//                svcs.remove(svc.getId());
//                if (!isAdd) svcs.put(oldSvc.getId(),oldSvc);
//                logger.error("Couldn't apply service point " + svc.getId() + " ", e);
//                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, svc.getId(), e);
//            }
//        } finally {
//            oldSvc = null;
//        }
//        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", messageText + svc.getId()));
//    }

    public synchronized void createUpdateMetaEntity(String user, MetaEndpoint meta, MetaEndpoint oldMeta,
                                                      boolean isAdd, SCAGAppContext appContext, int storeType)
                                                      throws SCAGJspException {
        String messageText = "";
        Map map = (storeType == Constants.STORE_TYPE_CENTER)? getMetaCenters(): getMetaServices();
        try {
            if (isAdd) {
                messageText = "Added new meta point: ";
                logger.error("SAVE MEP, id=" + meta.getId() );
                appContext.getScag().invokeCommand("addMetaEntity", meta, appContext, this, configFilename);
            } else {
                messageText = "Changed meta point: ";
                logger.error("UPDATE MEP, id=" + meta.getId() );
                appContext.getScag().invokeCommand("updateMetaEntity", meta, appContext, this, configFilename);
            }
//            appContext.getScag().invokeCommand("add_Svc", null, appContext, this, configFilename);
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                map.remove(meta.getId());
                if (!isAdd) map.put(oldMeta.getId(), oldMeta);
                logger.error("Couldn't apply meta " + meta.getId() + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, meta.getId(), e);
            }
        } finally {
            oldMeta = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Meta Point", messageText + meta.getId()));
    }

public synchronized void updateMetaEndpoint(
        String user, MetaEndpoint meta, String key, SCAGAppContext appContext, boolean add
        )throws SCAGJspException {

        String messageText = "";
        try {
            if (add) {
                messageText = "Add meta ep: ";
                logger.error("UPDATE MEP, id=" + meta.getId() + " | " + key );
                String[] pair = new String[] {meta.getId(), key };
                appContext.getScag().invokeCommand("addMetaEndpoint", pair, appContext, this, configFilename);
            } else {
                messageText = "Removed meta ep: ";
                logger.error("REMOVE MEP, id=" + meta.getId() + " | " + key );
                String[] pair = new String[] {meta.getId(), key };
                appContext.getScag().invokeCommand("removeMetaEndpoint", pair, appContext, this, configFilename);
            }
//            appContext.getScag().invokeCommand("add_Svc", null, appContext, this, configFilename);
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
//                map.remove(meta.getId());
//                if (!isAdd) map.put(oldMeta.getId(), oldMeta);
                logger.error("Couldn't " + messageText + " " + meta.getId() + " ", e);
//                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, meta.getId(), e);
            }
        } finally {
//            oldMeta = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Meta Point", messageText + meta.getId()));
    }

    public PrintWriter store(final PrintWriter out) {

        final List svcsValues = new LinkedList(svcs.values());
        Collections.sort(svcsValues, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final Svc c1 = (Svc) o1;
                final Svc c2 = (Svc) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });

        final List ceterValues = new LinkedList(centers.values());
        Collections.sort(ceterValues, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final Center c1 = (Center) o1;
                final Center c2 = (Center) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });

        final List metaServiceValues = new LinkedList(metaServices.values());
        Collections.sort(metaServiceValues, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final MetaEndpoint c1 = (MetaEndpoint) o1;
                final MetaEndpoint c2 = (MetaEndpoint) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });

        final List metaCenterValues = new LinkedList(metaCenters.values());
        Collections.sort(metaCenterValues, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final MetaEndpoint c1 = (MetaEndpoint) o1;
                final MetaEndpoint c2 = (MetaEndpoint) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });

        Functions.storeConfigHeader(out, "records", "smpp.dtd", Functions.getLocaleEncoding());
        storeUid(out, getLastUsedId());

        for (Iterator iterator = svcsValues.iterator(); iterator.hasNext();)
            ((Svc) iterator.next()).store(out);
        for (Iterator iterator = ceterValues.iterator(); iterator.hasNext();)
            ((Center) iterator.next()).store(out);
        for (Iterator iterator = metaServiceValues.iterator(); iterator.hasNext();)
            ((MetaEndpoint) iterator.next()).store(out, Constants.STORE_TYPE_SVC);
        for (Iterator iterator = metaCenterValues.iterator(); iterator.hasNext();)
            ((MetaEndpoint) iterator.next()).store(out, Constants.STORE_TYPE_CENTER);

        Functions.storeConfigFooter(out, "records");
        return out;
    }

    public synchronized Map getSvcs() {
        return svcs;
    }

    public synchronized List getSvcsNames() {
        return new SortedList(svcs.keySet());
    }

    public synchronized Map getSvcsStatuses(SCAGAppContext appContext) {
        Map result = new HashMap();
        try {
            List svcList = appContext.getScag().getSmeInfo();
            for (Iterator iterator = svcList.iterator(); iterator.hasNext();) {
                Map status = ru.sibinco.scag.util.Utils.stringToMap((String) iterator.next(), ",");
                SvcStatus svcStatus = new SvcStatus(status);
                result.put(svcStatus.getId(), svcStatus);
            }

        } catch (SibincoException e) {
            List c = getSvcsNames();
            for (Iterator i = c.iterator(); i.hasNext();) {
                SvcStatus svcStatus = new SvcStatus((String) i.next());
                result.put(svcStatus.getId(), svcStatus);
            }

        } catch (NullPointerException e) {
            logger.error("Could not get SCAG daemon");
        }
        return result;
    }

    public synchronized Map getCenterStatuses(SCAGAppContext appContext) {
        Map result = new HashMap();
        try {
            List centerList = appContext.getScag().getSmscInfo();
            for (Iterator iterator = centerList.iterator(); iterator.hasNext();) {
                Map status = ru.sibinco.scag.util.Utils.stringToMap((String) iterator.next(), ",");
                CenterStatus centerStatus = new CenterStatus(status);
                result.put(centerStatus.getId(), centerStatus);
            }

        } catch (SibincoException e) {
            List c = getCenterNames();

            for (Iterator i = c.iterator(); i.hasNext();) {
                CenterStatus centerStatus = new CenterStatus((String) i.next());
                result.put(centerStatus.getId(), centerStatus);
            }

        }catch (NullPointerException e) {
            logger.error("Could not get SCAG daemon");
        }
        return result;
    }

    public synchronized Map getCenters() {
        return centers;
    }

//    public synchronized Map getMetaEndpoints() {
//        return metaEndpoints;
//    }

    public synchronized Map getMetaCenters() {
        return metaCenters;
    }

    public synchronized Map getMetaServices() {
        return metaServices;
    }

    private PrintWriter storeUid(final PrintWriter out, int lastUid) {
        out.println("<param_conf name=\"last used uid\" type=\"int\">" + lastUid + "</param_conf>");
        return out;
    }

    public synchronized List getCenterNames() {
        return new SortedList(centers.keySet());
    }

    public void store() throws SibincoException {
        try {
            store(new PrintWriter(new FileWriter(configFilename))).close();
        } catch (IOException e) {
            logger.error("Couldn't save Svc's config", e);
            throw new SibincoException("Couldn't save Svc's config", e);
        }
    }

    public synchronized int getLastUsedId() {
        return lastUsedId;
    }

    public synchronized void setLastUsedId(int lastUsedId) {
        this.lastUsedId = lastUsedId;
    }

    public Map getServicesByMetaGroupId( String id ){
        Map selectedMap = new HashMap();
        for( Iterator iter = getSvcs().keySet().iterator(); iter.hasNext(); ){
            String key = (String)iter.next();
            Svc svc = (Svc)getSvcs().get( key );
            if( svc.getMetaGroup().equals(id) ){
                selectedMap.put( key, svc );
            }
        }
        return selectedMap;
    }

    public Map getCentersByMetaGroupId( String id ){
        Map selectedMap = new HashMap();
        for( Iterator iter = getCenters().keySet().iterator(); iter.hasNext(); ){
            String key = (String)iter.next();
            Center center = (Center)getCenters().get( key );
            if( center.getMetaGroup().equals(id) ){
                selectedMap.put( key, center );
            }
        }
        return selectedMap;
    }

}
