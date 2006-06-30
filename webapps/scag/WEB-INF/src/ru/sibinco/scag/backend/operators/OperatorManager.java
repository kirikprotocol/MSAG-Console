/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.operators;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import javax.xml.parsers.ParserConfigurationException;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.io.FileOutputStream;

import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

/**
 * The <code>OperatorManager</code> class represents
 * <p><p/>
 * Date: 02.03.2006
 * Time: 13:56:46
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class OperatorManager {

    private Logger logger = Logger.getLogger(this.getClass());
    private final Map operators = Collections.synchronizedMap(new TreeMap());
    private long lastUsedOperatorId = -1;
    private final String configFilename;
    private static final String PARAM_NAME_LAST_USED_OPERATOR_ID = "last used operator id";


    public OperatorManager(final String configFilename) {
        this.configFilename = configFilename;
    }

    public synchronized void init() throws IOException, ParserConfigurationException, SAXException {
        operators.clear();
        if (configFilename != null) {
            final Document document = Utils.parse(configFilename);
            final NodeList paramConfs = document.getDocumentElement().getElementsByTagName("param");
            final NodeList operatorsRecords = document.getDocumentElement().getElementsByTagName("operator");
            for (int i = 0; i < paramConfs.getLength(); i++) {
                final Element paramElem = (Element) paramConfs.item(i);
                final String name = paramElem.getAttribute("name");
                try {
                    if (PARAM_NAME_LAST_USED_OPERATOR_ID.equals(name)) {
                        lastUsedOperatorId = Integer.decode(Utils.getNodeText(paramConfs.item(i))).intValue();
                    }
                } catch (NumberFormatException e) {
                    logger.error("Int parameter \"" + name + "\" misformatted: " + lastUsedOperatorId + ", skipped", e);
                }
            }

            for (int i = 0; i < operatorsRecords.getLength(); i++) {
                final Element element = (Element) operatorsRecords.item(i);
                final String id = element.getAttribute("id");
                Operator operator = new Operator(Long.valueOf(id));
                Node node = operatorsRecords.item(i);
                NodeList nodeList = node.getChildNodes();
                for (int j = 0; j < nodeList.getLength(); j++) {
                    Node operatorNode = nodeList.item(j);
                    if (operatorNode.getNodeName().equalsIgnoreCase("name")) {
                        operator.setName(Utils.getNodeText(operatorNode));
                    }
                    if (operatorNode.getNodeName().equalsIgnoreCase("description")) {
                        operator.setDescription(Utils.getNodeText(operatorNode));
                    }
                }

                final NodeList masks = element.getElementsByTagName("mask");

                for (int j = 0; j < masks.getLength(); j++) {
                    Mask mask = null;
                    Node maskNode = masks.item(j);
                    if (maskNode.getNodeName().equalsIgnoreCase("mask")) {
                        try {
                            mask = new Mask(Utils.getNodeText(maskNode));
                            operator.getMasks().put(mask.getMask(), mask);
                        } catch (SibincoException e) {
                            e.printStackTrace();
                        }
                    }
                }
                operators.put(operator.getId(), operator);
            }
        }
    }

    public synchronized void updateOperator(final String user, final long id, final String name,
                                            final String description, final String[] srcMasks)
            throws NullPointerException, SibincoException {

        final Operator operator = (Operator) operators.get(new Long(id));
        if (null == operator)
            throw new NullPointerException("Operator \"" + id + "\" not found.");
        operator.setName(name);
        if (description != null)
            operator.setDescription(description);
        operator.getMasks().clear();
        for (int i = 0; i < srcMasks.length; i++) {
            final String srcMask = srcMasks[i];
            if (null != srcMask && 0 < srcMask.trim().length()) {
                final Mask mask = new Mask(srcMask);
                operator.getMasks().put(mask.getMask(), mask);
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Operator", "Changed operator: " + name + "."));
    }

    public synchronized long createOperator(final String user, final String name, final String description, final String[] srcMasks)
            throws NullPointerException, SibincoException {
        final Operator operator = new Operator(new Long(++lastUsedOperatorId), name);
        operator.setDescription(description);
        operators.put(operator.getId(), operator);
        operator.getMasks().clear();
        for (int i = 0; i < srcMasks.length; i++) {
            final String srcMask = srcMasks[i];
            if (null != srcMask && 0 < srcMask.trim().length()) {
                final Mask mask = new Mask(srcMask);
                operator.getMasks().put(mask.getMask(), mask);
            }
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Operator", "Added new operator: " + name + "."));
        return getLastUsedOperatorId();
    }

    private void storeOperators() throws IOException {
        File configFile = new File(configFilename);
        File configNew = Functions.createNewFilenameForSave(configFile);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "operators", "operators.dtd", System.getProperty("file.encoding").equals("Cp1251") ? "ISO-8859-1" : System.getProperty("file.encoding"));
        out.print(getParamXmlText());
        for (Iterator i = new SortedList(operators.keySet()).iterator(); i.hasNext();) {
            Long id = (Long) i.next();
            Operator operator = (Operator) operators.get(id);
            out.print(getOperatorXmlText(operator));
        }
        Functions.storeConfigFooter(out, "operators");
        out.flush();
        out.close();
        Functions.renameNewSavedFileToOriginal(configNew, configFile);
    }

    private String getParamXmlText() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("   <param name=\"" + PARAM_NAME_LAST_USED_OPERATOR_ID + "\" type=\"int\">").
                append(getLastUsedOperatorId()).append("</param>").append("\n");
        return buffer.toString();
    }

    private String getOperatorXmlText(Operator operator) {
        StringBuffer buffer = new StringBuffer();
        Map masks = operator.getMasks();

        buffer.append("   <operator id=\"").append(operator.getId()).append("\">").append("\n");
        buffer.append("       <name>").append(StringEncoderDecoder.encode(operator.getName().trim())).append("</name>").append("\n");
        buffer.append("       <description>").append(StringEncoderDecoder.encode((operator.getDescription() == null) ? "" : operator.getDescription().trim())).append("</description>").append("\n");
        buffer.append("       <masks>").append("\n");
        if (masks != null) {
            for (Iterator i = new SortedList(masks.keySet()).iterator(); i.hasNext();) {
                String id = (String) i.next();
                Mask mask = (Mask) masks.get(id);
                buffer.append("        <mask>").append(mask.getMask()).append("</mask>").append("\n");
            }
        }
        buffer.append("       </masks>").append("\n");
        buffer.append("   </operator>").append("\n");

        return buffer.toString();
    }

    public synchronized void delete(final String user, final ArrayList toRemove) throws SCAGJspException {
        List operatorNames = getOperatorNamesByIds(toRemove);
        getOperators().keySet().removeAll(toRemove);
        StatusManager.getInstance().addStatMessages(new StatMessage(user, "Operator", "Deleted operator(s): "
                + operatorNames.toString() + "."));
    }



    public synchronized void reloadOperators(final Scag scag) throws SCAGJspException {
        try {
            scag.reloadOperators();
        } catch (SibincoException e) {
            if (Proxy.STATUS_CONNECTED == scag.getStatus()) {
                 logger.error("Couldn't reload Operator ", e);
                throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
            }
        } finally {
            try {
                storeOperators();
            } catch (IOException e) {
                logger.debug("Couldn't save config", e);
            }
        }
    }

    private List getOperatorNamesByIds(final ArrayList operatorsIds) {
        List result = new ArrayList();
        for (Iterator i = new SortedList(operators.keySet()).iterator(); i.hasNext();) {
            Long id = (Long) i.next();
            Operator operator = (Operator) operators.get(id);
            for (Iterator it = operatorsIds.iterator(); it.hasNext();) {
               Long operatorId = (Long) it.next();
               if(operator.getId().equals(operatorId)){
                   result.add(operator.getName());
               }
            }
        }
        return result;
    }


    public long getLastUsedOperatorId() {
        return lastUsedOperatorId;
    }

    public synchronized Map getOperators() {
        return operators;
    }
}
