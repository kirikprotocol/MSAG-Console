package ru.novosoft.smsc.admin.cluster_controller;

/**
 * @author Artem Snopkov
 */
public class CCRouteTrace {
  private final String routeId;
  private final String [] trace;

  public CCRouteTrace(String routeId, String[] trace) {
    this.routeId = routeId;
    this.trace = trace;
  }

  public String getRouteId() {
    return routeId;
  }

  public String[] getTrace() {
    return trace;
  }
}
