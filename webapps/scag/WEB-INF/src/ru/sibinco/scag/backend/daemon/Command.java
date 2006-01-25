/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.apache.log4j.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.SibincoException;

import java.util.Collection;

public class Command {

    protected final Document document;
    protected final Logger logger = Logger.getLogger(this.getClass());
    private final String systemId;

    public Command(final String commandName, final String systemId) throws SibincoException {
        try {
            this.systemId = systemId;
            final DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            document = builder.newDocument();
            final Element elem = document.createElement("command");
            elem.setAttribute("name", StringEncoderDecoder.encode(commandName));
            document.appendChild(elem);
        } catch (Throwable e) {
            logger.error("Couldn't create command \"" + commandName + "\"", e);
            throw new SibincoException("Couldn't create command \"" + commandName + "\"", e);
        }
    }

    protected Command(final String commandName) throws SibincoException {
        this(commandName, "file:///command.dtd");
    }

    public Document getDocument() {
        return document;
    }

    public String getSystemId() {
        return systemId;
    }

    protected void createStringParam(final String name, final String value) {
        createParam(name, "string", value);
    }

    protected void createIntParam(final String name, final long value) {
        createParam(name, "int", String.valueOf(value));
    }

    protected void createBoolParam(final String name, final boolean value) {
        createParam(name, "bool", String.valueOf(value));
    }

    protected void createStringListParam(final String name, final Collection value) {
        createParam(name, "stringlist", Functions.collectionToString(value, ","));
    }

    private void createParam(final String name, final String type, final String value) {
        final Element element = document.createElement("param");
        element.setAttribute("name", name);
        element.setAttribute("type", type);
        element.appendChild(document.createTextNode(value));
        document.getDocumentElement().appendChild(element);
    }
}
