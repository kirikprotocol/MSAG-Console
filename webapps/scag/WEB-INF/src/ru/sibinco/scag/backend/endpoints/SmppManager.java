/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.sme.ProviderManager;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileWriter;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.LinkedList;
import java.util.Comparator;
import java.util.Iterator;


/**
 * The <code>SmppManager</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 12:39:13
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppManager {

    private static final String PARAM_NAME_LAST_UID_ID = "last used uid";

    private Logger logger = Logger.getLogger(this.getClass());

    private final Map svcs = Collections.synchronizedMap(new HashMap());
    private final Map centers = Collections.synchronizedMap(new HashMap());
    private final String configFilename;
    private final ProviderManager providerManager;

    private int lastUidId = -1;


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
                        lastUidId = Integer.decode(Utils.getNodeText(paramConfs.item(i))).intValue();
                    }
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                    logger.error("Int parameter \"" + name + "\" misformatted: " + lastUidId + ", skipped", e);
                }
                System.out.println("lastUidId=" + lastUidId);
            }

            final NodeList smerecords = document.getDocumentElement().getElementsByTagName("smerecord");
            for (int i = 0; i < smerecords.getLength(); i++) {
                final Element svcRecords = (Element) smerecords.item(i);
                final Svc service = createSvc(svcRecords);
                svcs.put(service.getId(), service);
            }
            final NodeList ceterRecords = document.getDocumentElement().getElementsByTagName("smscrecord");
            for (int i = 0; i < ceterRecords.getLength(); i++) {
                final Element centersRecords = (Element) ceterRecords.item(i);
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

    public synchronized PrintWriter store(final PrintWriter out) {
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
        storeUid(out, getLastUidId());
        for (Iterator iterator = svcsValues.iterator(); iterator.hasNext();)
            ((Svc) iterator.next()).store(out);
        for (Iterator iterator = ceterValues.iterator(); iterator.hasNext();)
            ((Center) iterator.next()).store(out);
        Functions.storeConfigFooter(out, "records");
        return out;
    }

    public Map getSvcs() {
        return svcs;
    }

    public synchronized List getSvcsNames() {
        return new SortedList(svcs.keySet());
    }

    public Map getCenters() {
        return centers;
    }

    public PrintWriter storeUid(final PrintWriter out, int lastUid) {
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

    public int getLastUidId() {
        return lastUidId;
    }

    public void setLastUidId(int lastUidId) {
        this.lastUidId = lastUidId;
    }
}
