package ru.novosoft.smsc.admin.cluster_controller;

/**
 * @author Artem Snopkov
 */
public class CCRouteTrace {
  private final String routeId;
  private final String [] trace;
  private final String aliasInfo;

  public CCRouteTrace(String routeId, String[] trace, String aliasInfo) {
    this.routeId = routeId;
    this.trace = trace;
    this.aliasInfo = aliasInfo;
  }

  public String getRouteId() {
    return routeId;
  }

  public String[] getTrace() {
    return trace;
  }

  public String getAliasInfo() {
    return aliasInfo;
  }
}
