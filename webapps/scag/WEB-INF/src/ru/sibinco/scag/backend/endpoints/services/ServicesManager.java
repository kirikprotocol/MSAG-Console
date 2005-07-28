/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.endpoints.services;

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
 * The <code>ServicesManager</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:28:24
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServicesManager {

    private Logger logger = Logger.getLogger(this.getClass());

    private final Map services = Collections.synchronizedMap(new HashMap());
    private final String configFilename;

    public ServicesManager() {
        configFilename = null; //ToDo
    }

    public ServicesManager(String configFilename) {
        this.configFilename = configFilename;
    }

public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        services.clear();
        if (configFilename != null) {   //ToDo
            final Document document = Utils.parse(configFilename);
            final NodeList records = document.getDocumentElement().getElementsByTagName("services"); //ToDo: if need chenge this element tag
            for (int i = 0; i < records.getLength(); i++) {
                final Element servicesRecords = (Element) records.item(i);
                final Services service = createServices(servicesRecords);
                services.put(service.getId(), service);
            }
        }
    }

    protected Services createServices(Element servicesRecords) {
        return new Services(servicesRecords);
    }

    public synchronized PrintWriter store(final PrintWriter out) {
        final List values = new LinkedList(services.values());
        Collections.sort(values, new Comparator() {
            public int compare(final Object o1, final Object o2) {
                final Services c1 = (Services) o1;
                final Services c2 = (Services) o2;
                return c1.getId().compareTo(c2.getId());
            }
        });
        Functions.storeConfigHeader(out, "records", "ServicessRecords.tdt");//ToDo
        for (Iterator iterator = values.iterator(); iterator.hasNext();)
            ((Services) iterator.next()).store(out);
        Functions.storeConfigFooter(out, "records"); //ToDo
        return out;
    }

    public Map getServicess() {
        return services;
    }

    public synchronized List getServicesNames() {
        return new SortedList(services.keySet());
    }

    public void store() throws SibincoException {
        try {
            store(new PrintWriter(new FileWriter(configFilename))).close();
        } catch (IOException e) {
            logger.error("Couldn't save Services's config", e);
            throw new SibincoException("Couldn't save Services's config", e);
        }
    }
}