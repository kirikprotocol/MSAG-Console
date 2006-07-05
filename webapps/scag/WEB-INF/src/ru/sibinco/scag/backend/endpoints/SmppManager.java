/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.Manager;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileWriter;
import java.util.*;


/**
 * The <code>SmppManager</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 12:39:13
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppManager implements Manager {

    private static final String PARAM_NAME_LAST_UID_ID = "last used uid";

    private Logger logger = Logger.getLogger(this.getClass());

    private final Map svcs = Collections.synchronizedMap(new HashMap());
    private final Map centers = Collections.synchronizedMap(new HashMap());
    private final String configFilename;
    private final ProviderManager providerManager;

    private int lastUsedId = -1;


    public SmppManager(String configFilename, ProviderManager providerManager) {
        this.configFilename = configFilename;
        this.providerManager = providerManager;
    }                                                                     

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        svcs.clear();
        centers.clear();
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

            final NodeList smerecords = document.getDocumentElement().getElementsByTagName("smerecord");
            for (int i = 0; i < smerecords.getLength(); i++) {
                final Element svcRecords = (Element) smerecords.item(i);
                final Svc service = createSvc(svcRecords);
                svcs.put(service.getId(), service);
            }
            final NodeList centerRecords = document.getDocumentElement().getElementsByTagName("smscrecord");
            for (int i = 0; i < centerRecords.getLength(); i++) {
                final Element centersRecords = (Element) centerRecords.item(i);
                final Center center = createCenter(centersRecords);
                centers.put(center.getId(), center);
            }
        }
    }

    private Svc createSvc(Element svcRecords) {
        return new Svc(svcRecords, providerManager);
    }

    protected Center createCenter(Element centersRecords) {
        return new Center(centersRecords, providerManager);
    }

    public synchronized void deleteCenters(final String user, final Set checkedSet, final Scag scag) throws SCAGJspException {
        final Map centers = getCenters();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String centerId = (String) iterator.next();
            Center center = (Center) centers.get(centerId);
            centers.remove(centerId);
            try {
                if (center.isEnabled()) {
                    scag.invokeCommand("deleteCenter",center,scag,this,configFilename);
                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete Smsc \"" + centerId + '"', e);
                    centers.put(centerId,center);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, centerId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", "Deleted center(s): "
                + checkedSet.toString() + "."));
    }

    public synchronized void deleteServicePoints(final String user, final Set checkedSet, final Scag scag) throws SCAGJspException {
        final Map svcs = getSvcs();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String svcId = (String) iterator.next();
            Svc svc = (Svc) svcs.get(svcId);
            svcs.remove(svcId);
            try {
                if (svc.isEnabled()) {
                    scag.invokeCommand("deleteSvc",svcId,scag,this,configFilename);
                }
            } catch (SibincoException e) {
                if (!(e instanceof StatusDisconnectedException)) {
                    logger.error("Couldn't delete sme \"" + svcId + '"', e);
                    svcs.put(svcId,svc);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, svcId, e);
                }
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", "Deleted service point(s): "
                + checkedSet.toString() + "."));
    }

    public synchronized void createUpdateCenter(String user, boolean isAdd, boolean isEnabled, Center center,
                                                Scag scag, Center oldCenter) throws SCAGJspException {
        String messageText = "";
        try {
            if (isAdd) {
                messageText = "Added new center: ";
                int uid = getLastUsedId();
                center.setUid(++uid);
                setLastUsedId(center.getUid());
                if (center.isEnabled()) {
                    scag.invokeCommand("addCenter",center,scag,this,configFilename);
                }

            } else {
                messageText = "Changed center: ";
                if (oldCenter.isEnabled() == center.isEnabled()) {
                    if (isEnabled)
                        scag.invokeCommand("updateCenter",center,scag,this,configFilename);
                } else {
                    if (center.isEnabled()) {
                        scag.invokeCommand("addCenter",center,scag,this,configFilename);
                    } else {
                        //TODO why delete???
                        //scag.invokeCommand("deleteCenter",center,scag,this,configFilename);
                        HashSet set = new HashSet();
                        set.add(center.getId());
                        deleteCenters(user,set,scag);
                    }
                }
            }
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                centers.remove(center.getId());
                if (!isAdd) centers.put(oldCenter.getId(),oldCenter);
                logger.error("Couldn't applay Centers " + center.getId() + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, center.getId(), e);
            }
        }
        finally {
            oldCenter = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Center", messageText + center.getId()));
    }

    public synchronized void createUpdateServicePoint(String user, Svc svc,
                                                      boolean isAdd,
                                                      boolean isEnabled, Scag scag,
                                                      Svc oldSvc) throws SCAGJspException {
        String messageText = "";
        try {
            if (isAdd) {
                messageText = "Added new service point: ";
                if (svc.isEnabled()) {
                    scag.invokeCommand("addSvc", svc, scag, this, configFilename);
                }
            } else {
                messageText = "Changed service point: ";
                if ((oldSvc.isEnabled() == svc.isEnabled())) {
                    if (isEnabled)
                        scag.invokeCommand("updateSvcInfo", svc, scag, this, configFilename);
                } else {
                    if (svc.isEnabled()) {
                        scag.invokeCommand("addSvc", svc, scag, this, configFilename);
                    } else {
                        //TODO why delete???
                        //scag.deleteSvc(svc.getId());
                        HashSet set = new HashSet();
                        set.add(svc.getId());
                        deleteServicePoints(user,set,scag);
                    }
                }
            }
        } catch (SibincoException e) {
            if (!(e instanceof StatusDisconnectedException)) {
                svcs.remove(svc.getId());
                if (!isAdd) svcs.put(oldSvc.getId(),oldSvc);
                logger.error("Couldn't applay service point " + svc.getId() + " ", e);
                throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, svc.getId(), e);
            }
        } finally {
            oldSvc = null;
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Service Point", messageText + svc.getId()));
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
        Functions.storeConfigHeader(out, "records", "smpp.dtd", "ISO-8859-1");
        storeUid(out, getLastUsedId());
        for (Iterator iterator = svcsValues.iterator(); iterator.hasNext();)
            ((Svc) iterator.next()).store(out);
        for (Iterator iterator = ceterValues.iterator(); iterator.hasNext();)
            ((Center) iterator.next()).store(out);
        Functions.storeConfigFooter(out, "records");
        return out;
    }

    public synchronized Map getSvcs() {
        return svcs;
    }

    public synchronized List getSvcsNames() {
        return new SortedList(svcs.keySet());
    }

    public synchronized Map getCenters() {
        return centers;
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
}
