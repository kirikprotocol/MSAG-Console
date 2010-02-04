package ru.sibinco.scag.backend.stat.counters;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

import java.io.*;
import java.util.Map;
import java.util.HashMap;

import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;

import javax.xml.parsers.ParserConfigurationException;

/**
 * Copyright (c) EyeLine Communications
 * All rights reserved.
 * <p/>
 * User: makar
 * Date: 04.02.2010
 * Time: 10:33:20
 */
public class CountersManager
{
    private Logger logger = Logger.getLogger(this.getClass());

    private final HashMap<String, Counter> counters = new HashMap<String, Counter>();
    private final HashMap<String, CATable> ca_tables = new HashMap<String, CATable>();

    private final File configFile;
    private final HSDaemon hsDaemon;

    public CountersManager(String config, HSDaemon hsDaemon) throws IOException, ParserConfigurationException, SAXException
    {
        this.configFile = new File(config);
        this.hsDaemon = hsDaemon;
        try {
            load();
        } catch (IOException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw e;
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw e;
        } catch (SAXException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw e;
        }
    }

    private void load() throws IOException, ParserConfigurationException, SAXException
    {
        Document document = Utils.parse(new FileReader(configFile));
        NodeList counterNodes = document.getElementsByTagName("template");
        for (int i = 0; i < counterNodes.getLength(); i++) {
            Counter counter = parseXMLCounter(counterNodes.item(i));
            counters.put(counter.getId(), counter);
        }
        NodeList ca_tablesNodes = document.getElementsByTagName("ca_table");
        for (int i = 0; i < ca_tablesNodes.getLength(); i++) {
            CATable ca_table = parseXMLCATable(ca_tablesNodes.item(i));
            ca_tables.put(ca_table.getId(), ca_table);
            ca_tablesNodes.item(i);
        }
    }

    public synchronized void apply() throws IOException, ParserConfigurationException, SAXException, SibincoException
    {
        store();
        hsDaemon.store(configFile);
    }

    protected void store() throws IOException
    {
        File configNew = Functions.createNewFilenameForSave(configFile);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "counters", "counters.dtd", Functions.getLocaleEncoding());
        out.println("\t<templates>");
        for (Counter counter : counters.values()) out.print(getXMLText(counter));
        out.println("\t</templates>");
        out.println("\t<ca_tables>\n");
        for (CATable ca_table: ca_tables.values()) out.print(getXMLText(ca_table));
        out.println("\t</ca_tables>\n");
        Functions.storeConfigFooter(out, "counters");
        out.flush(); out.close();

        Functions.renameNewSavedFileToOriginal(configNew, configFile);
    }

    private String getXMLText(Counter counter)
    {
        final String header =
            "\t\t<template id=\"" + StringEncoderDecoder.encode(counter.getId()) + '"' +
            " type=\"" + StringEncoderDecoder.encode(counter.getType()) + "\">\n" +
            "\t\t\t<ca id=\"" + StringEncoderDecoder.encode(counter.getCATableId()) + "\"/>\n";
        // TODO: dump additional params
        String footer = "\t\t</template>\n";
        return header + footer;
    }
    private Counter parseXMLCounter(Node node)
    {
        NamedNodeMap attributes = node.getAttributes();
        String id = attributes.getNamedItem("id").getNodeValue();
        String type = attributes.getNamedItem("type").getNodeValue();
        String ca_table_id = node.getChildNodes().item(0).getAttributes().getNamedItem("id").getNodeValue();

        final Counter counter = new Counter(id, CounterType.valueOf(type));
        counter.setCATableId(ca_table_id);
        // TODO: check all
        return counter;
    }

    private String getXMLText(CATable ca_table)
    {
        final String header =
            "\t\t<ca_table id=\"" + StringEncoderDecoder.encode(ca_table.getId()) + '"' +
            " system=\"" + StringEncoderDecoder.encode(ca_table.getSystem()) + "\">\n";
        String footer =
            "\t\t\t<limits min=\"" + StringEncoderDecoder.encode(ca_table.getLimitsMin()) + '"' +
            "max=\"" + StringEncoderDecoder.encode(ca_table.getLimitsMax())+ "\">\n";

        // TODO: dump limits content & additional params

        footer += "\t\t\t</limits>\n";
        footer += "\t\t</ca_table>\n";
        return header + footer;
    }
    private CATable parseXMLCATable(Node node)
    {
        // TODO: parse XML
        return new CATable("567", false);
    }

    public synchronized Map getCounters() {
        return counters;
    }

    public synchronized Map getCATables() {
        return ca_tables;
    }
}
