package ru.sibinco.smsx.stats;


/**
 * User: artem
 * Date: 20.10.11
 */
public interface RecordHandler {

  void handleRecord(String route, String sAddr, String dAddr, String srcSme, String srcMsc, int serviceId);
}
