package ru.sibinco.otasme.engine.smscenters;

import ru.sibinco.otasme.engine.smscenters.parser.RoutesParser;
import ru.sibinco.otasme.engine.smscenters.parser.SMSCentersParser;

import java.util.Collection;


/**
 * User: artem
 * Date: Aug 2, 2006
 */

final class SMSCentersXMLParser {

  public static SMSCenters parse(final String timezonesXML, final String routesXML) {
    // Read smscenters from timezonesXML
    final SMSCentersParser smscentersParser = new SMSCentersParser();
    smscentersParser.parse(ClassLoader.getSystemResourceAsStream(timezonesXML));
    final Collection smscenters = smscentersParser.getTimezones();

    // Add masks to smscenters from routesXML
    RoutesParser.parse(ClassLoader.getSystemResourceAsStream(routesXML), smscenters);

    return new SMSCenters(smscenters, smscentersParser.getDefaultSMSCenter());
  }

}
