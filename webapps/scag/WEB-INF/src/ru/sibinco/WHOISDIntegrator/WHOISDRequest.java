package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.transport.Transport;

import java.util.HashMap;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 10.03.2006
 * Time: 11:24:47
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDRequest {
  public static final byte OPERATORS = 0;
  public static final byte SERVICES = 1;
  public static final byte RULE = 2;
  public static final byte TARIFF_MATRIX = 3;
  public static final byte OPERATORS_SCHEMA = 4;
  public static final byte SERVICES_SCHEMA = 5;
  public static final byte TARIFF_MATRIX_SCHEMA = 6;
  public static final HashMap transportToXSl = new HashMap();
  public static final String SMPP_XSL = "smpp_rule.xsl";
  public static final String HTTP_XSL = "http_rule.xsl";
  public static final String MMS_XSL = "mms_rule.xsl";
  private int id;
  private String name;

  public static String[] xslFILES = { SMPP_XSL, HTTP_XSL, MMS_XSL };

  static {
     for(int i = 0; i<Transport.transportTitles.length;i++) {
       transportToXSl.put(Transport.transportTitles[i], xslFILES[i]);
     }
  }

  public static final WHOISDRequest[] WHOISDRequests = {
     new WHOISDRequest(OPERATORS,"operators.xml"),
     new WHOISDRequest(SERVICES,"providers.xml"),
     new WHOISDRequest(RULE,"rule.xml"),
     new WHOISDRequest(TARIFF_MATRIX,"tariffs.xml"),
     new WHOISDRequest(OPERATORS_SCHEMA,"operators.dtd"),
     new WHOISDRequest(SERVICES_SCHEMA,"services.dtd"),
     new WHOISDRequest(TARIFF_MATRIX_SCHEMA,"tariffs.dtd")
  };

  public WHOISDRequest(int id, String file) {
    this.id = id;
    this.name = file;
  }

  public static int getId(String requestedFile) {
    for (byte i = 0; i<WHOISDRequests.length; i++) {
      if (WHOISDRequests[i].getName().equals(requestedFile)) return WHOISDRequests[i].getId();
    }
    return -1;
  }

  public static String getSchemaName(int id) {
    for (byte i = 0; i<WHOISDRequests.length; i++) {
      if (WHOISDRequests[i].getId()==id) return WHOISDRequests[i].getName();
    }
    return null;
  }

  public int getId() {
    return id;
  }

  public String getName() {
    return name;
  }

  public String toString() {
    return name;
  }
}
