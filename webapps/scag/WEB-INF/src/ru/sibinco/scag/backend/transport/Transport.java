/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.transport;

/**
 * The <code>Transport</code> class represents
 * <p><p/>
 * Date: 27.09.2005
 * Time: 13:14:38
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Transport {

    public static final String SMPP_TRANSPORT_NAME = "SMPP";
    public static final String HTTP_TRANSPORT_NAME = "HTTP";
    public static final String MMS_TRANSPORT_NAME = "MMS";

    public static final int SMPP_TRANSPORT_ID = 1;
    public static final int HTTP_TRANSPORT_ID = 2;
    public static final int MMS_TRANSPORT_ID = 3;

    public static final String SMPP_SCHEMA_NAME = "smpp_rules.xsd";
    public static final String HTTP_SCHEMA_NAME = "http_rules.xsd";
    public static final String MMS_SCHEMA_NAME = "mms_rules.xsd";

    public static String[] transportIds = {
            String.valueOf(SMPP_TRANSPORT_ID),
            String.valueOf(HTTP_TRANSPORT_ID)
            /* Hide MMS 
            , String.valueOf(MMS_TRANSPORT_ID)  */
    };

    public static String[] transportTitles = {
            SMPP_TRANSPORT_NAME, HTTP_TRANSPORT_NAME/* Hide MMS , MMS_TRANSPORT_NAME*/};

    public static String[] schemaNames = {
            SMPP_SCHEMA_NAME, HTTP_SCHEMA_NAME/* Hide MMS , MMS_SCHEMA_NAME*/};

    public static int getTransportId(final String transport) {
        int Id = -1;
        for (int i = 0; i < transportTitles.length; i++) {
            String title = transportTitles[i];
            if (title.equals(transport)) {
                Id = i;
                break;
            }
        }
        return Id;
    }

    public static String getSchemaByTransport(final String transport) {
        String schema = null;
        for (int i = 0; i < transportTitles.length; i++) {
            String title = transportTitles[i];
            if (title.equals(transport)) {
                schema = schemaNames[i];
                break;
            }
        }
        return schema;
    }

    public static String getTransportName(int transortId) {
        switch (transortId) {
            case SMPP_TRANSPORT_ID:
                return SMPP_TRANSPORT_NAME;

            case HTTP_TRANSPORT_ID:
                return HTTP_TRANSPORT_NAME;

            case MMS_TRANSPORT_ID:
                return MMS_TRANSPORT_NAME;
            default:
                return null;
        }
    }
}
