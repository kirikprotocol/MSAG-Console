/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.centers;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import javax.xml.parsers.ParserConfigurationException;
import java.util.Map;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.LinkedList;
import java.util.Comparator;
import java.util.Iterator;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileWriter;

import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>CenterManager</code> class represents  of Center's Manager
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:36:33
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CenterManager {

    private Logger logger = Logger.getLogger(this.getClass());

    private final Map centers = Collections.synchronizedMap(new HashMap());
    private final String configFilename;

    public CenterManager(String configFilename) {
        this.configFilename = configFilename;
    }

    public CenterManager() {
        configFilename = null;//ToDo
    }

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        centers.clear();
        if (configFilename != null) {   //ToDo
            final Document document = Utils.parse(configFilename);
            final NodeList records = document.getDocumentElement().getElementsByTagName("centers"); //ToDo: if need chenge this element tag
            for (int i = 0; i < records.getLength(); i++) {
                final Element centersRecords = (Element) records.item(i);
                final Center center = createCenter(centersRecords);
                centers.put(center.getId(), center);
            }
        }
    }

    protected Center createCenter(Element centersRecords) {
        return new Center(centersRecords);
    }

    public synchronized PrintWriter store(final PrintWriter out) {
        final List values = new LinkedList(centers.values());
        Collections.sort(values, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final Center c1 = (Center) o1;
                final Center c2 = (Center) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });
        Functions.storeConfigHeader(out, "records", "CentersRecords.tdt");//ToDo
        for (Iterator iterator = values.iterator(); iterator.hasNext();)
            ((Center) iterator.next()).store(out);
        Functions.storeConfigFooter(out, "records"); //ToDo
        return out;
    }

    public Map getCenters() {
        return centers;
    }

    public synchronized List getCenterNames() {
        return new SortedList(centers.keySet());
    }

    public void store() throws SibincoException {
        try {
            store(new PrintWriter(new FileWriter(configFilename))).close();
        } catch (IOException e) {
            logger.error("Couldn't save Center's config", e);
            throw new SibincoException("Couldn't save Center's config", e);
        }
    }
}
