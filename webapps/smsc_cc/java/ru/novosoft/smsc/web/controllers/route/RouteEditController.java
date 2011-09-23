package ru.novosoft.smsc.web.controllers.route;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class RouteEditController extends RouteController {

  private Route route;
  private String sourceSubjectToAdd;
  private String destinationSubjectToAdd;
  private String destinationSmeToAdd;

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
    for (String smeId : getSmeIds())
      result.add(new SelectItem(smeId));
    return result;
  }

  public List<SelectItem> getSmesWithEmpty() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem(null, ""));
    result.addAll(getSmes());
    return result;    
  }

  public List<String> getSmeIds() {
    try {
      List<String> smeIds = new ArrayList<String>(WebContext.getInstance().getSmeManager().smes().keySet());
      Collections.sort(smeIds);
      return smeIds;
    } catch (AdminException e) {
      addError(e);
      return new ArrayList<String>();
    }
  }

  public List<SelectItem> getAcls() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    try {
      result.add(new SelectItem(null, ""));
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
      result.add(new SelectItem(null, ""));
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
      result.add(new SelectItem(null, ""));
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
    result.add(new SelectItem(null, ""));
    for (BillingMode m : BillingMode.values())
      result.add(new SelectItem(m, m.name()));
    return result;
  }

  public List<SelectItem> getTrafficModes() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (TrafficMode m : TrafficMode.values())
      result.add(new SelectItem(m, m.name()));
    return result;
  }


  public DynamicTableModel getSourceMasks() {
    DynamicTableModel m = new DynamicTableModel();
    for (Source src : route.getSources()) {
      if (src.getMask() != null) {
        DynamicTableRow tableRow = new DynamicTableRow();
        tableRow.setValue("mask", src.getMask().getSimpleAddress());
        m.addRow(tableRow);
      }
    }
    return m;
  }

  public void setSourceMasks(DynamicTableModel m) throws AdminException {
    List<Source> sources = route.getSources();
    if (sources == null) {
      sources = new ArrayList<Source>();
    } else {
      for (Iterator<Source> iter = sources.iterator(); iter.hasNext();) {
        Source s = iter.next();
        if (s.getMask() != null)
          iter.remove();
      }
    }

    for (DynamicTableRow r : m.getRows())
      sources.add(new Source(new Address((String) r.getValue("mask"))));

    route.setSources(sources);
  }

  public DynamicTableModel getDestinationMasks() {
    DynamicTableModel m = new DynamicTableModel();
    for (Destination dst : route.getDestinations()) {
      if (dst.getMask() != null) {
        DynamicTableRow tableRow = new DynamicTableRow();
        tableRow.setValue("mask", dst.getMask().getSimpleAddress());
        tableRow.setValue("sme", dst.getSmeId());
        m.addRow(tableRow);
      }
    }
    return m;
  }

  public void setDestinationMasks(DynamicTableModel m) throws AdminException {
    List<Destination> destinations = route.getDestinations();
    if (destinations == null) {
      destinations = new ArrayList<Destination>();
    } else {
      for (Iterator<Destination> iter = destinations.iterator(); iter.hasNext();) {
        Destination s = iter.next();
        if (s.getMask() != null)
          iter.remove();
      }
    }

    for (DynamicTableRow r : m.getRows())
      destinations.add(new Destination(new Address((String) r.getValue("mask")), (String) r.getValue("sme")));

    route.setDestinations(destinations);
  }

  public List<SelectItem> getSourceSubjects2Add() {
    List<Subject> subjects = getSettings().getSubjects();
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (Subject s : subjects) {
      if (!route.containsSource(s.getName()))
        result.add(new SelectItem(s.getName()));
    }
    return result;
  }

  public List<String> getSourceSubjects() {
    List<String> result = new ArrayList<String>();
    if (route.getSources() != null) {
      for (Source dst : route.getSources())
        if (dst.getSubject() != null)
          result.add(dst.getSubject());
    }
    return result;
  }

  public String getSourceSubjectToAdd() {
    return sourceSubjectToAdd;
  }

  public void setSourceSubjectToAdd(String sourceSubjectToAdd) {
    this.sourceSubjectToAdd = sourceSubjectToAdd;
  }

  public String addSourceSubject() {
    if (sourceSubjectToAdd != null) {
      try {
        route.addSource(new Source(sourceSubjectToAdd));
        save();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String removeSourceSubject() {
    String sourceSubject2remove = getRequestParameter("sourceSubject2remove");
    if (sourceSubject2remove != null) {
      route.removeSource(sourceSubject2remove);
      save();
    }
    return null;
  }


  /**
   * Возвращает список субъектов, которые могут быть добавлены в destinations
   *
   * @return список субъектов, которые могут быть добавлены в destinations
   */
  public List<SelectItem> getDestinationSubjects2Add() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    for (Subject s : getSettings().getSubjects()) {
      if (!route.containsDestination(s.getName()))
        result.add(new SelectItem(s.getName()));
    }
    return result;
  }

  public List<Destination> getDestinations() {
    List<Destination> destinations = route.getDestinations();
    for (Iterator<Destination> iter = destinations.iterator(); iter.hasNext();)
      if (iter.next().getSubject() == null)
        iter.remove();
    return destinations;
  }

  public String getDestinationSubjectToAdd() {
    return destinationSubjectToAdd;
  }

  public void setDestinationSubjectToAdd(String destinationSubjectToAdd) {
    this.destinationSubjectToAdd = destinationSubjectToAdd;
  }

  public String getDestinationSmeToAdd() {
    return destinationSmeToAdd;
  }

  public void setDestinationSmeToAdd(String destinationSmeToAdd) {
    this.destinationSmeToAdd = destinationSmeToAdd;
  }

  public String addDestinationSubject() {
    if (destinationSubjectToAdd != null && destinationSmeToAdd != null) {
      try {
        route.addDestination(new Destination(destinationSubjectToAdd, destinationSmeToAdd));
        save();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String removeDestinationSubject() {
    String destinationSubject2remove = getRequestParameter("destinationSubject2remove");
    if (destinationSubject2remove != null)
      route.removeDestination(destinationSubject2remove);
    save();
    return null;
  }

  public Route getRoute() {
    return route;
  }

  public Long getProvider() {
    return route.getProviderId();
  }

  public void setProvider(Long provider) {
    if (provider == 0)
      provider = null;
    route.setProviderId(provider);
  }

  public Long getCategory() {
    return route.getCategoryId();
  }

  public void setCategory(Long category) {
    if (category == 0)
      category = null;
    route.setCategoryId(category);
  }

  public String getSrcSme() {
    return route.getSrcSmeId();
  }

  public void setSrcSme(String sme) {
    if (sme != null && sme.equals(""))
      sme = null;
    route.setSrcSmeId(sme);
  }

  public String getBackupSme() {
    return route.getBackupSmeId();
  }

  public void setBackupSme(String sme) {
    if (sme != null && sme.equals(""))
      sme = null;
    route.setBackupSmeId(sme);
  }

  public Integer getAcl() {
    return route.getAclId();
  }

  public void setAcl(Integer acl) {
    if (acl == 0)
      acl = null;
    route.setAclId(acl);
  }

  public String save() {
    RouteSubjectSettings s = getSettings();
    List<Route> routes = s.getRoutes();
    for (Iterator<Route> iter = routes.iterator(); iter.hasNext();) {
      if (iter.next().getName().equals(route.getName())) {
        iter.remove();
        break;
      }
    }
    routes.add(route);

    try {
      s.setRoutes(routes);
      setSettings(s);
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    return "ROUTES";
  }

}
