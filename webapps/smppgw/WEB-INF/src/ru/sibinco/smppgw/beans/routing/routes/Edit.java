package ru.sibinco.smppgw.beans.routing.routes;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.SmppGWAppContext;
import ru.sibinco.smppgw.backend.routing.GwRoute;
import ru.sibinco.smppgw.backend.routing.TrafficRules;
import ru.sibinco.smppgw.backend.sme.Provider;
import ru.sibinco.smppgw.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;


/**
 * Created by igork
 * Date: 20.04.2004
 * Time: 15:51:47
 */
public class Edit extends EditBean
{
  private String name;
  private String[] srcMasks;
  private String[] srcSubjs;
  private String[] dstMasks;
  private String[] dstSubjs;
  private int priority;
  private boolean enabling;
  private boolean archiving;
  private boolean billing;
  private int serviceId;
  private boolean suppressDeliveryReports;
  private boolean active;
  private String srcSmeId;
  private String deliveryMode;
  private String forwardTo;
  private boolean hide;
  private boolean forceReplayPath;
  private String notes;

  private String provider;
  private long providerId;

  private Set srcMasksSet = new HashSet();
  private Set srcSubjsSet = new HashSet();
  private Set dstMasksSet = new HashSet();
  private Set dstSubjectsSet = new HashSet();

  private String dst_mask_sme_;
  private Map destinations;
  private static final String DST_SME_PREFIX = "dst_sme_";
  private static final String DST_MASK_PREFIX = "dst_mask_sme_";

  private boolean trafficRules_allowReceive = false;
  private boolean trafficRules_allowAnswer = false;
  private String trafficRules_sendLimit = "";//=U
  private boolean trafficRules_allowPssrResp = false;
  private boolean trafficRules_allowUssrRequest = false;
  private String trafficRules_ussdMiDialogLimit = "";
  private boolean trafficRules_allowUssdDialogInit = false;
  private String trafficRules_ussdSiDialogLimit = "";

  public String getId()
  {
    return name;
  }

  public void process(HttpServletRequest request, HttpServletResponse response) throws SmppgwJspException
  {
    appContext = (SmppGWAppContext) request.getAttribute("appContext");

    destinations = new HashMap();
    for (Iterator i = request.getParameterMap().entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      final String s = (String) entry.getKey();
      if (s.startsWith(DST_SME_PREFIX)) {
        String subjName = s.substring(DST_SME_PREFIX.length());
        String[] smeNameStrings = (String[]) entry.getValue();
        StringBuffer smeName = new StringBuffer();
        for (int j = 0; j < smeNameStrings.length; j++) {
          String smeNameString = smeNameStrings[j];
          if (smeNameString != null)
            smeName.append(smeNameString.trim());
        }
        final Subject subj = (Subject) appContext.getGwRoutingManager().getSubjects().get(subjName);
        if (subj == null)
          throw new SmppgwJspException(Constants.errors.routing.routes.DEST_SUBJ_NOT_FOUND, subjName);
        final String smeId = smeName.toString();
        final Sme sme = (Sme) appContext.getGwSmeManager().getSmes().get(smeId);
        if (sme == null)
          throw new SmppgwJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);

        try {
          final Destination destination = new Destination(subj, sme);
          destinations.put(destination.getName(), destination);
        } catch (SibincoException e) {
          logger.debug("Could not create destination", e);
          throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
        }
      } else if (s.startsWith(DST_MASK_PREFIX)) {
        String maskName = s.substring(DST_MASK_PREFIX.length());
        if (maskName != null && maskName.trim().length() > 0) {
          String[] smeNameStrings = (String[]) entry.getValue();
          StringBuffer smeName = new StringBuffer();
          for (int j = 0; j < smeNameStrings.length; j++) {
            String smeNameString = smeNameStrings[j];
            if (smeNameString != null)
              smeName.append(smeNameString.trim());
          }
          Mask mask = null;
          try {
            mask = new Mask(maskName);
          } catch (SibincoException e) {
            logger.debug("Could not create destination mask", e);
            throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, maskName, e);
          }
          if (mask == null)
            throw new SmppgwJspException(Constants.errors.routing.routes.DEST_SUBJ_NOT_FOUND, maskName);
          final String smeId = smeName.toString();
          final Sme sme = (Sme) appContext.getGwSmeManager().getSmes().get(smeId);
          if (sme == null)
            throw new SmppgwJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);

          try {
            final Destination destination = new Destination(mask, sme);
            destinations.put(destination.getName(), destination);
          } catch (SibincoException e) {
            logger.debug("Could not create destination", e);
            throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
          }
        }
      }
    }
    super.process(request, response);
  }

  protected void load(String loadId) throws SmppgwJspException
  {
    if (dst_mask_sme_ == null && appContext.getGwSmeManager().getSmes().size() > 0)
      dst_mask_sme_ = (String) appContext.getGwSmeManager().getSmes().keySet().iterator().next();

    final GwRoute route = (GwRoute) appContext.getGwRoutingManager().getRoutes().get(loadId);
    if (route != null) {
      name = route.getName();

      List subjList = new ArrayList();
      List maskList = new ArrayList();
      for (Iterator i = route.getSources().values().iterator(); i.hasNext();) {
        Source source = (Source) i.next();
        if (source.isSubject()) {
          subjList.add(source.getName());
          srcSubjsSet.add(source.getName());
        } else {
          maskList.add(source.getName());
          srcMasksSet.add(source.getName());
        }
      }
      srcMasks = (String[]) maskList.toArray(new String[0]);
      srcSubjs = (String[]) subjList.toArray(new String[0]);

      maskList.clear();
      subjList.clear();

      for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
        Destination destination = (Destination) i.next();
        if (destination.isSubject()) {
          subjList.add(destination.getName());
          dstSubjectsSet.add(destination.getName());
        } else {
          maskList.add(destination.getName());
          dstMasksSet.add(destination.getName());
        }
      }
      dstMasks = (String[]) maskList.toArray(new String[0]);
      dstSubjs = (String[]) subjList.toArray(new String[0]);

      priority = route.getPriority();
      enabling = route.isEnabling();
      archiving = route.isArchiving();
      billing = route.isBilling();
      serviceId = route.getServiceId();
      suppressDeliveryReports = route.isSuppressDeliveryReports();
      active = route.isActive();
      srcSmeId = route.getSrcSmeId();
      System.out.println("srcSmeId = " + srcSmeId);
      deliveryMode = route.getDeliveryMode();
      forwardTo = route.getForwardTo();
      hide = route.isHide();
      forceReplayPath = route.isForceReplayPath();
      notes = route.getNotes();

      if (route.getProvider() != null) {
        provider = route.getProvider().getName();
        providerId = route.getProvider().getId();
      }

      final TrafficRules trafficRules = route.getTrafficRules();
      if (trafficRules != null) {
        trafficRules_allowReceive = trafficRules.isAllowReceive();
        trafficRules_allowAnswer = trafficRules.isAllowAnswer();
        trafficRules_sendLimit = trafficRules.getSendLimitStr();
        trafficRules_allowPssrResp = trafficRules.isAllowPssrResp();
        trafficRules_allowUssrRequest = trafficRules.isAllowUssrRequest();
        trafficRules_ussdMiDialogLimit = String.valueOf(trafficRules.getUssdMiDialogLimit());
        trafficRules_allowUssdDialogInit = trafficRules.isAllowUssdDialogInit();
        trafficRules_ussdSiDialogLimit = String.valueOf(trafficRules.getUssdSiDialogLimit());
      }
    }
  }

  protected void save() throws SmppgwJspException
  {
    Map routes = appContext.getGwRoutingManager().getRoutes();

    try {
      Map sources = createSources();
      Provider providerObj = (Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));

      final TrafficRules trafficRules = new TrafficRules(trafficRules_allowReceive, trafficRules_allowAnswer, trafficRules_sendLimit,
                                                         trafficRules_allowPssrResp, trafficRules_allowUssrRequest, trafficRules_ussdMiDialogLimit,
                                                         trafficRules_allowUssdDialogInit, trafficRules_ussdSiDialogLimit);
      if (isAdd()) {
        if (routes.containsKey(name))
          throw new SmppgwJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, name);
        routes.put(name,
                   new GwRoute(name, priority, enabling, billing, archiving, suppressDeliveryReports, active, serviceId,
                               (Map) sources, destinations, srcSmeId, deliveryMode, forwardTo, hide, forceReplayPath,
                               notes, providerObj, trafficRules));
      } else {
        if (!getEditId().equals(name) && routes.containsKey(name))
          throw new SmppgwJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
        routes.remove(getEditId());
        routes.put(name, new GwRoute(name, priority, enabling, billing, archiving, suppressDeliveryReports, active, serviceId,
                                     (Map) sources, destinations, srcSmeId, deliveryMode, forwardTo, hide, forceReplayPath,
                                     notes, providerObj, trafficRules));
      }
    } catch (SibincoException e) {
      logger.debug("Could not create new subject", e);
      throw new SmppgwJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
    }
    throw new DoneException();
  }

  private Map createSources() throws SibincoException
  {
    Map result = new HashMap();
    for (int i = 0; i < srcSubjs.length; i++) {
      String srcSubj = srcSubjs[i];
      Subject subject = (Subject) appContext.getGwRoutingManager().getSubjects().get(srcSubj);
      if (subject != null) {
        final Source source = new Source(subject);
        result.put(source.getName(), source);
      }
    }
    for (int i = 0; i < srcMasks.length; i++) {
      String srcMask = srcMasks[i];
      if (srcMask != null && srcMask.trim().length() > 0) {
        Mask mask = new Mask(srcMask);
        Source source = new Source(mask);
        result.put(source.getName(), source);
      }
    }
    return result;
  }

  public String[] getSmeIds()
  {
    return (String[]) new SortedList(appContext.getGwSmeManager().getSmes().keySet()).toArray(new String[0]);
  }

  public String[] getProviderIds()
  {
    Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    ArrayList result = new ArrayList(providers.size());
    for (Iterator i = providers.keySet().iterator(); i.hasNext();) {
      result.add(String.valueOf(((Long) i.next()).longValue()));
    }
    return (String[]) result.toArray(new String[0]);
  }

  public String[] getProviders()
  {
    Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    ArrayList result = new ArrayList(providers.size());
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      result.add(((Provider) i.next()).getName());
    }
    return (String[]) result.toArray(new String[0]);
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String[] getSrcMasks()
  {
    return srcMasks;
  }

  public void setSrcMasks(String[] srcMasks)
  {
    this.srcMasks = srcMasks;
  }

  public String[] getSrcSubjs()
  {
    return srcSubjs;
  }

  public void setSrcSubjs(String[] srcSubjs)
  {
    this.srcSubjs = srcSubjs;
  }

  public String[] getDstMasks()
  {
    return dstMasks;
  }

  public void setDstMasks(String[] dstMasks)
  {
    this.dstMasks = dstMasks;
  }

  public String[] getDstSubjs()
  {
    return dstSubjs;
  }

  public void setDstSubjs(String[] dstSubjs)
  {
    this.dstSubjs = dstSubjs;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(int priority)
  {
    this.priority = priority;
  }

  public boolean isEnabling()
  {
    return enabling;
  }

  public void setEnabling(boolean enabling)
  {
    this.enabling = enabling;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(boolean archiving)
  {
    this.archiving = archiving;
  }

  public boolean isBilling()
  {
    return billing;
  }

  public void setBilling(boolean billing)
  {
    this.billing = billing;
  }

  public int getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(int serviceId)
  {
    this.serviceId = serviceId;
  }

  public boolean isSuppressDeliveryReports()
  {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getDeliveryMode()
  {
    return deliveryMode;
  }

  public void setDeliveryMode(String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(boolean hide)
  {
    this.hide = hide;
  }

  public boolean isForceReplayPath()
  {
    return forceReplayPath;
  }

  public void setForceReplayPath(boolean forceReplayPath)
  {
    this.forceReplayPath = forceReplayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(String notes)
  {
    this.notes = notes;
  }

  public String getProvider()
  {
    return provider;
  }

  public void setProvider(String provider)
  {
    this.provider = provider;
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(long providerId)
  {
    this.providerId = providerId;
  }

  public List getAllSmes()
  {
    return new SortedList(appContext.getGwSmeManager().getSmes().keySet());
  }

  public List getAllUncheckedSrcSubjects()
  {
    final SortedList srcSubjs = new SortedList(appContext.getGwRoutingManager().getSubjects().keySet());
    srcSubjs.removeAll(srcSubjsSet);
    return srcSubjs;
  }

  public Map getAllUncheckedDstSubjects()
  {
    Map result = new TreeMap();
    for (Iterator i = appContext.getGwRoutingManager().getSubjects().values().iterator(); i.hasNext();) {
      Subject subject = (Subject) i.next();
      result.put(subject.getName(), subject.getDefaultSme().getId());
    }
    return result;
  }

  public Map getDstSubjPairs()
  {
    final GwRoute route = (GwRoute) appContext.getGwRoutingManager().getRoutes().get(getEditId());
    if (route != null) {
      Map result = new TreeMap();
      for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
        Destination destination = (Destination) i.next();
        if (destination.isSubject())
          result.put(destination.getName(), destination.getSme().getId());
      }
      return result;
    }
    return null;
  }

  public Map getDstMaskPairs()
  {
    final GwRoute route = (GwRoute) appContext.getGwRoutingManager().getRoutes().get(getEditId());
    if (route != null) {
      Map result = new TreeMap();
      for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
        Destination destination = (Destination) i.next();
        if (!destination.isSubject())
          result.put(destination.getName(), destination.getSme().getId());
      }
      return result;
    }
    return null;
  }

  public String getDst_mask_sme_()
  {
    return dst_mask_sme_;
  }

  public void setDst_mask_sme_(String dst_mask_sme_)
  {
    this.dst_mask_sme_ = dst_mask_sme_;
  }

  public boolean isTrafficRules_allowReceive()
  {
    return trafficRules_allowReceive;
  }

  public void setTrafficRules_allowReceive(boolean trafficRules_allowReceive)
  {
    this.trafficRules_allowReceive = trafficRules_allowReceive;
  }

  public boolean isTrafficRules_allowAnswer()
  {
    return trafficRules_allowAnswer;
  }

  public void setTrafficRules_allowAnswer(boolean trafficRules_allowAnswer)
  {
    this.trafficRules_allowAnswer = trafficRules_allowAnswer;
  }

  public String getTrafficRules_sendLimit()
  {
    return trafficRules_sendLimit;
  }

  public void setTrafficRules_sendLimit(String trafficRules_sendLimit)
  {
    this.trafficRules_sendLimit = trafficRules_sendLimit;
  }

  public boolean isTrafficRules_allowPssrResp()
  {
    return trafficRules_allowPssrResp;
  }

  public void setTrafficRules_allowPssrResp(boolean trafficRules_allowPssrResp)
  {
    this.trafficRules_allowPssrResp = trafficRules_allowPssrResp;
  }

  public boolean isTrafficRules_allowUssrRequest()
  {
    return trafficRules_allowUssrRequest;
  }

  public void setTrafficRules_allowUssrRequest(boolean trafficRules_allowUssrRequest)
  {
    this.trafficRules_allowUssrRequest = trafficRules_allowUssrRequest;
  }

  public String getTrafficRules_ussdMiDialogLimit()
  {
    return trafficRules_ussdMiDialogLimit;
  }

  public void setTrafficRules_ussdMiDialogLimit(String trafficRules_ussdMiDialogLimit)
  {
    this.trafficRules_ussdMiDialogLimit = trafficRules_ussdMiDialogLimit;
  }

  public boolean isTrafficRules_allowUssdDialogInit()
  {
    return trafficRules_allowUssdDialogInit;
  }

  public void setTrafficRules_allowUssdDialogInit(boolean trafficRules_allowUssdDialogInit)
  {
    this.trafficRules_allowUssdDialogInit = trafficRules_allowUssdDialogInit;
  }

  public String getTrafficRules_ussdSiDialogLimit()
  {
    return trafficRules_ussdSiDialogLimit;
  }

  public void setTrafficRules_ussdSiDialogLimit(String trafficRules_ussdSiDialogLimit)
  {
    this.trafficRules_ussdSiDialogLimit = trafficRules_ussdSiDialogLimit;
  }
}
