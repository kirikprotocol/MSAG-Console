package ru.novosoft.smsc.web.controllers.route_trace;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.RouteTrace;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RouteTraceController extends SettingsMController<RouteSubjectSettings> {

  private Address sourceAddress;
  private Address destinationAddress;
  private String sourceSmeId;
  private RouteTrace trace;
  private boolean initFailed;
  private boolean showTrace;
  private boolean routeFound;

  public RouteTraceController() {
    super(WebContext.getInstance().getRouteSubjectManager());

    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
    }

    showTrace = getRequestParameter("showTrace") != null;
    routeFound = getRequestParameter("routeFound") != null;
  }

  public boolean isInitFailed() {
    return initFailed;
  }

  public String getSourceAddress() {
    if (sourceAddress == null)
      return null;
    return sourceAddress.getSimpleAddress();
  }

  public void setSourceAddress(String sourceAddress) {
    if (sourceAddress != null && sourceAddress.length() > 0)
      this.sourceAddress = new Address(sourceAddress);
    else
      this.sourceAddress = null;
  }

  public String getDestinationAddress() {
    if (destinationAddress == null)
      return null;
    return destinationAddress.getSimpleAddress();
  }

  public void setDestinationAddress(String destinationAddress) {
    if (destinationAddress != null && destinationAddress.length() > 0)
      this.destinationAddress = new Address(destinationAddress);
    else
      this.destinationAddress = null;
  }

  public String getSourceSmeId() {
    return sourceSmeId;
  }

  public void setSourceSmeId(String sourceSmeId) {
    this.sourceSmeId = sourceSmeId;
  }

  public boolean isShowTrace() {
    return showTrace;
  }

  public boolean isRouteFound() {
    return routeFound;
  }

  public RouteTrace getTrace() {
    return trace;
  }

  public List<SelectItem> getSmes() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      List<String> smeIds = new ArrayList<String>(WebContext.getInstance().getSmeManager().smes().keySet());
      Collections.sort(smeIds);
      for (String smeId : smeIds)
        result.add(new SelectItem(smeId));
    } catch (AdminException e) {
      addError(e);
    }
    return result;
  }

  public String getTraceLog() {
    if (trace == null)
      return null;

    StringBuilder sb = new StringBuilder();
    if (trace.getAliasInfo() != null)
      sb.append(trace.getAliasInfo()).append("\n\n");

    if (trace.getTrace() != null) {
      for (String s : trace.getTrace())
        sb.append(s).append("\n");
    }

    return sb.toString();
  }

  public String traceRoute() {
    RouteSubjectSettings settings = getSettings();
    try {
      trace = settings.traceRoute(sourceAddress, destinationAddress, sourceSmeId);
      showTrace = true;
      routeFound = trace != null && trace.getRouteName() != null;
    } catch (AdminException e) {
      addError(e);
    }

    return null;
  }
}
