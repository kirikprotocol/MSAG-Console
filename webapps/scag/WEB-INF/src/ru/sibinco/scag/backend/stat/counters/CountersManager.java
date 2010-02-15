package ru.sibinco.scag.backend.stat.counters;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;

import java.io.*;
import java.util.HashMap;
import java.util.Collection;

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
        }
    }

    public synchronized void apply() throws IOException, ParserConfigurationException, SAXException, SibincoException
    {
        store();
        hsDaemon.store(configFile);
    }

    public void addCounter(Counter counter) {
        counters.put(counter.getId(), counter);
        // TODO: apply config, command call to MSAG, restore if failed
    }
    public void addCATable(CATable ca_table) {
        ca_tables.put(ca_table.getId(), ca_table);
        // TODO: apply config, command call to MSAG, restore if failed
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

    private void parseParam(ConfigParamOwner owner, Node node)
    {
        NamedNodeMap nodeAttributes = node.getAttributes();
        final String paramName = nodeAttributes.getNamedItem("name").getNodeValue();
        final String paramType = nodeAttributes.getNamedItem("type").getNodeValue();
        String paramValue = "";
        NodeList paramValueList = node.getChildNodes();
        for (int e = 0; e < paramValueList.getLength(); e++) {
          if (paramValueList.item(e).getNodeType() == Node.TEXT_NODE) {
              Node cNode = paramValueList.item(e);
              if (cNode != null) paramValue += cNode.getNodeValue();
          }
        }
        logger.debug("param=" + paramName + ", type=" + paramType + ", value=" + paramValue);
        owner.setParam(paramName, paramType, paramValue);
    }

    private String getXMLText(Counter counter)
    {
        String text =
            "\t\t<template id=\"" + StringEncoderDecoder.encode(counter.getId()) + '"' +
            " type=\"" + StringEncoderDecoder.encode(counter.getTypeString()) + "\">\n" +
            "\t\t\t<ca id=\"" + StringEncoderDecoder.encode(counter.getCATableId()) + "\"/>\n";

        final Collection<ConfigParam> params = counter.getParams();
        for (ConfigParam param : params) { // dump additional params
            text += "\t\t\t<param name=\"" + StringEncoderDecoder.encode(param.getName() + '"' +
                    " type=\"" + StringEncoderDecoder.encode(param.getType()) + "\">" +
                    StringEncoderDecoder.encode(param.getValue()) + "</param>\n");
        }

        text += "\t\t</template>\n";
        return text;
    }

    private Counter parseXMLCounter(Node node)
    {
        NamedNodeMap attributes = node.getAttributes();
        String id = attributes.getNamedItem("id").getNodeValue();
        String type = attributes.getNamedItem("type").getNodeValue();
        logger.debug("id=" + id + ", type=" + type);
        final Counter counter = new Counter(id, CounterType.valueOf(type));

        NodeList childs = node.getChildNodes();
        for (int i = 0; i < childs.getLength(); i++) {
            Node childNode = childs.item(i);
            String nodeName = childNode.getNodeName();
            if (nodeName.equals("ca")) {
                NamedNodeMap childNodeAttributes = childNode.getAttributes();
                final String ca_table_id = childNodeAttributes.getNamedItem("id").getNodeValue();
                logger.debug("ca_id=" + ca_table_id);
                counter.setCATableId(ca_table_id);
            } else if (nodeName.equals("param")) {
                parseParam(counter, childNode);
            }
        }
        return counter;
    }

    private String getXMLText(CATable ca_table)
    {
        String text =
            "\t\t<ca_table id=\"" + StringEncoderDecoder.encode(ca_table.getId()) + '"' +
            " system=\"" + StringEncoderDecoder.encode(ca_table.getSystem()) + "\">\n" +
            "\t\t\t<limits min=\"" + StringEncoderDecoder.encode(ca_table.getLimitsMinStr()) + '"' +
            " max=\"" + StringEncoderDecoder.encode(ca_table.getLimitsMaxStr())+ "\">\n";

        // TODO: dump limits content
        text += "\t\t\t</limits>\n";

        final Collection<ConfigParam> params = ca_table.getParams();
        for (ConfigParam param : params) { // dump additional params
            text += "\t\t\t<param name=\"" + StringEncoderDecoder.encode(param.getName() + '"' +
                    " type=\"" + StringEncoderDecoder.encode(param.getType()) + "\">" +
                    StringEncoderDecoder.encode(param.getValue()) + "</param>\n");
        }
        text += "\t\t</ca_table>\n";
        return text;
    }
    private CATable parseXMLCATable(Node node)
    {
        NamedNodeMap attributes = node.getAttributes();
        String id = attributes.getNamedItem("id").getNodeValue();
        String system = attributes.getNamedItem("system").getNodeValue();
        logger.debug("id=" + id + ", system=" + system);
        final CATable ca_table = new CATable(id, Boolean.valueOf(system));

        NodeList childs = node.getChildNodes();
        for (int i = 0; i < childs.getLength(); i++) {
            Node childNode = childs.item(i);
            final String nodeName = childNode.getNodeName();
            if (nodeName.equals("limits"))
            {
                final NamedNodeMap childNodeAttributes = childNode.getAttributes();
                int min = 0; int max = 100; // Default hardcoded values
                try { min = Integer.parseInt(childNodeAttributes.getNamedItem("min").getNodeValue()); }
                catch (Throwable th) { logger.warn("Failed to get 'min' attribute, using default=" + min); }
                try { max = Integer.parseInt(childNodeAttributes.getNamedItem("max").getNodeValue()); }
                catch (Throwable th) { logger.warn("Failed to get 'max' attribute, using default=" + max); }
                ca_table.setLimitsMin(min); ca_table.setLimitsMax(max);

                NodeList limits = childNode.getChildNodes();
                for (int j = 0; j < limits.getLength(); j++) {
                    Node limitNode = limits.item(j);
                    String limitNodeName = limitNode.getNodeName();
                    if (limitNodeName == null || !limitNodeName.equals("limit")) continue;
                    final NamedNodeMap limitAttributes = limitNode.getAttributes();
                    final String limitPercent = limitAttributes.getNamedItem("percent").getNodeValue();
                    final String limitSeverity = limitAttributes.getNamedItem("severity").getNodeValue();
                    logger.debug("Percent=" + limitPercent + ", severity=" + limitSeverity);
                    ca_table.addLimit(limitPercent, limitSeverity); 
                }
            }
            else if (nodeName.equals("param")) {
                parseParam(ca_table, childNode);
            }
        }
        return ca_table;
    }

    public synchronized HashMap<String, Counter> getCounters() {
        return counters;
    }

    public synchronized HashMap<String, CATable> getCATables() {
        return ca_tables;
    }
}
