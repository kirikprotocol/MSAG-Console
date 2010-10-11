package ru.novosoft.smsc.web.controllers.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RouteEditController extends RouteController {

  private Route route;

  public RouteEditController() {
    String routeName = getRequestParameter("route");
    if (routeName != null)
      route = getRouteByName(getSettings().getRoutes(), routeName);
    if (route == null)
      route = new Route();
  }

  private static Route getRouteByName(List<Route> routes, String name) {
    for (Route r : routes)
      if (r.getName().equals(name))
        return r;
    return null;
  }

  public List<SelectItem> getSmes() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      for (String smeId : WebContext.getInstance().getSmeManager().smes().keySet())
        result.add(new SelectItem(smeId));
    } catch (AdminException e) {
      addError(e);
    }
    return result;
  }

  public List<SelectItem> getAcls() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      for (Acl acl : WebContext.getInstance().getAclManager().acls())
        result.add(new SelectItem(acl.getId(), acl.getName()));
    } catch (AdminException e) {
      addError(e);
    }
    return result;
  }

  public List<SelectItem> getProviders() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      for (Provider p : WebContext.getInstance().getProviderManager().getSettings().getProviders())
        result.add(new SelectItem(p.getId(), p.getName()));
    } catch (AdminException e) {
      addError(e);
    }
    return result;
  }

  public List<SelectItem> getCategories() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      for (Category category : WebContext.getInstance().getCategoryManager().getSettings().getCategories())
        result.add(new SelectItem(category.getId(), category.getName()));
    } catch (AdminException e) {
      addError(e);
    }
    return result;
  }

  public List<SelectItem> getDeliveryModes() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (DeliveryMode m : DeliveryMode.values())
      result.add(new SelectItem(m, m.name()));
    return result;
  }

  public List<SelectItem> getReplayPaths() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (ReplayPath p : ReplayPath.values())
      result.add(new SelectItem(p, p.name()));
    return result;
  }

  public List<SelectItem> getBillingModes() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (BillingMode m : BillingMode.values())
      result.add(new SelectItem(m, m.name()));
    return result;
  }

  /**
   * Возвращает список субъектов, которые могут быть добавлены в sources
   * @return список субъектов, которые могут быть добавлены в sources
   */
  public List<SelectItem> getSourceSubjectsToAdd() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (Subject s : getSettings().getSubjects()) {
      boolean alreadyContains = false;
      for (Source src : route.getSources()) {
        if (src.getSubject() != null && src.getSubject().equals(s.getName())) {
          alreadyContains = true;
          break;
        }
      }
      if (!alreadyContains)
        result.add(new SelectItem(s.getName()));
    }
    return result;
  }

  /**
   * Возвращает список субъектов, которые могут быть добавлены в destinations
   * @return список субъектов, которые могут быть добавлены в destinations
   */
  public List<SelectItem> getDestinationSubjectsToAdd() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (Subject s : getSettings().getSubjects()) {
      boolean alreadyContains = false;
      for (Destination dst : route.getDestinations()) {
        if (dst.getSubject() != null && dst.getSubject().equals(s.getName())) {
          alreadyContains = true;
          break;
        }
      }
      if (!alreadyContains)
        result.add(new SelectItem(s.getName()));
    }
    return result;
  }


  public Route getRoute() {
    return route;
  }

  public String save() {
    return "ROUTES";
  }

}
