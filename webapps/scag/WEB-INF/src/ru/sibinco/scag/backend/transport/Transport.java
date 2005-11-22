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

    public static final String  SMPP_TRANSPORT_NAME = "SMPP";
    public static final String  WAP_TRANSPORT_NAME = "WAP";
    public static final String  MMS_TRANSPORT_NAME = "MMS";

    public static final int  SMPP_TRANSPORT_ID = 1;
    public static final int  WAP_TRANSPORT_ID = 2;
    public static final int  MMS_TRANSPORT_ID = 3;

    public static final String  SMPP_SCHEMA_NAME ="smpp_rules.xsd";
    public static final String  WAP_SCHEMA_NAME = "wap_rules.xsd";
    public static final String  MMS_SCHEMA_NAME = "mms_rules.xsd";

    public static String[] transportIds = {
            String.valueOf(SMPP_TRANSPORT_ID),  String.valueOf(WAP_TRANSPORT_ID), String.valueOf(MMS_TRANSPORT_ID)
        };

    public static String[] transportTitles = {
            SMPP_TRANSPORT_NAME,  WAP_TRANSPORT_NAME, MMS_TRANSPORT_NAME };

    public static String[] schemaNames = {
            SMPP_SCHEMA_NAME,  WAP_SCHEMA_NAME, MMS_SCHEMA_NAME  };

}
