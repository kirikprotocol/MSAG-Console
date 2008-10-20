/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.commands;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;

public class CommandWriter {

    protected final Logger logger = Logger.getLogger(getClass());

    private final OutputStream out;
    private final Transformer transformer;
    private final ByteArrayOutputStream outBuffer = new ByteArrayOutputStream();


    public CommandWriter(OutputStream outStream) throws SibincoException {
        out = outStream;
        try {
            transformer = TransformerFactory.newInstance().newTransformer();
        } catch (TransformerConfigurationException e) {
            logger.debug("Couldn't create XML Transormer", e);
            throw new SibincoException("Couldn't create XML Transormer", e);
        }

        transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
        transformer.setOutputProperty(OutputKeys.INDENT, "yes");
        transformer.setOutputProperty(OutputKeys.METHOD, "xml");
        transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");
    }

    public void write(Command command) throws SibincoException, IOException {
        logger.debug("CommandWriter.write() command " + command.getClass().getName() + " [" + command.getSystemId() + "]");
        final Document document = command.getDocument();
        try {
            outBuffer.reset();
            transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, command.getSystemId());
            transformer.transform(new DOMSource(document), new StreamResult(outBuffer));
            writeLength(outBuffer.size());
            outBuffer.writeTo(out);
            logger.debug("CommandWriter.write() Command:\n" + outBuffer.toString());
        } catch (TransformerException e) {
            logger.error("CommandWriter.write() TransformerException Couldn't process command", e);
            throw new SibincoException("Couldn't process command", e);
        }
    }

    protected void writeLength(int length)
            throws IOException {
        byte len[] = new byte[4];
        for (int i = 0; i < 4; i++) {
            len[3 - i] = (byte) (length & 0xff);
            length >>= 8;
        }

        out.write(len);
    }

}
