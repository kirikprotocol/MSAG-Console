/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.response;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.util.xml.Utils;

/**
 * The <code>Response</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 11:16:34
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Response {

    public static final byte STATUS_UNKNOWN = 0;
    public static final byte STATUS_ERROR = 1;
    public static final byte STATUS_OK = 2;

    protected final Document document;
    private final byte status;

    private final Logger logger = Logger.getLogger(this.getClass());

    private static final Status[] statuses = {
            new Status("Undefined", STATUS_UNKNOWN),
            new Status("Error", STATUS_ERROR),
            new Status("Ok", STATUS_OK)
    };

    public Response(Document document) {
        this.document = document;
        Element main = document.getDocumentElement();
        status = parseStatus(main.getAttribute("status"));
    }

    private byte parseStatus(String status) {
        for (int i = 0; i < statuses.length; i++)
            if (statuses[i].name.equalsIgnoreCase(status))
        return statuses[i].status;
        logger.warn("Unknown status \"" + status + "\" in response");
        return 0;
    }

    public static String getStatusString(byte status) {
        for (int i = 0; i < statuses.length; i++) {
            if (statuses[i].status == status)
                return statuses[i].name;
        }
        return "unknown";
    }

    public String getStatusString() {
        return getStatusString(status);
    }

    public Document getData() {
        return document;
    }

    public String getDataAsString() {
        String dataAsString = Utils.getNodeText(document.getDocumentElement());
        return dataAsString.startsWith("\n")?dataAsString.substring("\n".length(),dataAsString.length()):dataAsString;
    }

    public byte getStatus() {
        return status;
    }

    public String toString() {
        return "Response = " + getStatusString() + " [" + super.toString() + "]";
    }

    static class Status {
        String name;
        byte status = 0;

        public Status(String name, byte status) {
            this.name = name;
            this.status = status;
        }
    }
}
