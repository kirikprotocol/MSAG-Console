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
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends EditBean
{
  private String name;
  private String[] srcMasks = new String[0];
  private String[] srcSubjs = new String[0];
  private String[] dstMasks = new String[0];
  private String[] dstSubjs = new String[0];
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

  private String new_dstMask;
  private String new_dst_mask_sme_;
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

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SmppgwJspException
  {
    appContext = (SmppGWAppContext) request.getAttribute("appContext");

    destinations = new HashMap();
    for (Iterator i = request.getParameterMap().entrySet().iterator(); i.hasNext();) {
      final Map.Entry entry = (Map.Entry) i.next();
      final String s = (String) entry.getKey();
      if (s.startsWith(DST_SME_PREFIX)) {
        final String subjName = s.substring(DST_SME_PREFIX.length());
        final String[] smeNameStrings = (String[]) entry.getValue();
        final StringBuffer smeName = new StringBuffer();
        for (int j = 0; j < smeNameStrings.length; j++) {
          final String smeNameString = smeNameStrings[j];
          if (null != smeNameString)
            smeName.append(smeNameString.trim());
        }
        final Subject subj = (Subject) appContext.getGwRoutingManager().getSubjects().get(subjName);
        if (null == subj)
          throw new SmppgwJspException(Constants.errors.routing.routes.DEST_SUBJ_NOT_FOUND, subjName);
        final String smeId = smeName.toString();
        final Sme sme = (Sme) appContext.getGwSmeManager().getSmes().get(smeId);
        if (null == sme)
          throw new SmppgwJspException(Constants.errors.routing.routes.SME_NOT_FOUND, smeId);

        try {
          final Destination destination = new Destination(subj, sme);
          destinations.put(destination.getName(), destination);
        } catch (SibincoException e) {
          logger.debug("Could not create destination", e);
          throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
        }
      } else if (s.startsWith(DST_MASK_PREFIX)) {
        final String maskName = s.substring(DST_MASK_PREFIX.length());
        if (null != maskName && 0 < maskName.trim().length()) {
          final String[] smeNameStrings = (String[]) entry.getValue();
          final StringBuffer smeName = new StringBuffer();
          for (int j = 0; j < smeNameStrings.length; j++) {
            final String smeNameString = smeNameStrings[j];
            if (null != smeNameString)
              smeName.append(smeNameString.trim());
          }
          final Mask mask;
          try {
            mask = new Mask(maskName);
          } catch (SibincoException e) {
            logger.debug("Could not create destination mask", e);
            throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, maskName, e);
          }
          final String smeId = smeName.toString();
          final Sme sme = (Sme) appContext.getGwSmeManager().getSmes().get(smeId);
          if (null == sme)
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
    if (null != new_dstMask && null != new_dst_mask_sme_ && 0 < new_dstMask.length() && 0 < new_dst_mask_sme_.length()) {
      final Mask mask;
      try {
        mask = new Mask(new_dstMask);
      } catch (SibincoException e) {
        logger.debug("Could not create destination mask", e);
        throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION_MASK, new_dstMask, e);
      }
      final Sme sme = (Sme) appContext.getGwSmeManager().getSmes().get(new_dst_mask_sme_);
      if (null == sme)
        throw new SmppgwJspException(Constants.errors.routing.routes.SME_NOT_FOUND, new_dst_mask_sme_);
      try {
        final Destination destination = new Destination(mask, sme);
        destinations.put(destination.getName(), destination);
        if (0 < appContext.getGwSmeManager().getSmes().size())
          new_dst_mask_sme_ = (String) appContext.getGwSmeManager().getSmes().keySet().iterator().next();
        new_dstMask = "";
      } catch (SibincoException e) {
        logger.debug("Could not create destination", e);
        throw new SmppgwJspException(Constants.errors.routing.routes.COULD_NOT_CREATE_DESTINATION, e);
      }
    }
    super.process(request, response);
  }

  protected void load(final String loadId) throws SmppgwJspException
  {
    if (null == new_dst_mask_sme_ && 0 < appContext.getGwSmeManager().getSmes().size())
      new_dst_mask_sme_ = (String) appContext.getGwSmeManager().getSmes().keySet().iterator().next();

    final GwRoute route = (GwRoute) appContext.getGwRoutingManager().getRoutes().get(loadId);
    if (null != route) {
      name = route.getName();

      final List subjList = new ArrayList();
      final List maskList = new ArrayList();
      for (Iterator i = route.getSources().values().iterator(); i.hasNext();) {
        final Source source = (Source) i.next();
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
        final Destination destination = (Destination) i.next();
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
      deliveryMode = route.getDeliveryMode();
      forwardTo = route.getForwardTo();
      hide = route.isHide();
      forceReplayPath = route.isForceReplayPath();
      notes = route.getNotes();

      logger.debug("Load route: Provider: " + route.getProvider());
      if (null != route.getProvider()) {
        provider = route.getProvider().getName();
        providerId = route.getProvider().getId();
      }

      final TrafficRules trafficRules = route.getTrafficRules();
      if (null != trafficRules) {
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
    final Map routes = appContext.getGwRoutingManager().getRoutes();

    try {
      final Map sources = createSources();
      final Provider providerObj = (Provider) appContext.getProviderManager().getProviders().get(new Long(providerId));

      final TrafficRules trafficRules = new TrafficRules(trafficRules_allowReceive, trafficRules_allowAnswer, trafficRules_sendLimit,
                                                         trafficRules_allowPssrResp, trafficRules_allowUssrRequest, trafficRules_ussdMiDialogLimit,
                                                         trafficRules_allowUssdDialogInit, trafficRules_ussdSiDialogLimit);
      if (isAdd()) {
        if (routes.containsKey(name))
          throw new SmppgwJspException(Constants.errors.routing.routes.ROUTE_ALREADY_EXISTS, name);
        routes.put(name,
                   new GwRoute(name, priority, enabling, billing, archiving, suppressDeliveryReports, active, serviceId,
                               sources, destinations, srcSmeId, deliveryMode, forwardTo, hide, forceReplayPath,
                               notes, providerObj, trafficRules));
      } else {
        if (!getEditId().equals(name) && routes.containsKey(name))
          throw new SmppgwJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
        routes.remove(getEditId());
        routes.put(name, new GwRoute(name, priority, enabling, billing, archiving, suppressDeliveryReports, active, serviceId,
                                     sources, destinations, srcSmeId, deliveryMode, forwardTo, hide, forceReplayPath,
                                     notes, providerObj, trafficRules));
      }
    } catch (SibincoException e) {
      logger.error("Could not create new subject", e);
      throw new SmppgwJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
    }
    appContext.getStatuses().setRoutesChanged(true);
    throw new DoneException();
  }

  private Map createSources() throws SibincoException
  {
    final Map result = new HashMap();
    for (int i = 0; i < srcSubjs.length; i++) {
      final String srcSubj = srcSubjs[i];
      final Subject subject = (Subject) appContext.getGwRoutingManager().getSubjects().get(srcSubj);
      if (null != subject) {
        final Source source = new Source(subject);
        result.put(source.getName(), source);
      }
    }
    for (int i = 0; i < srcMasks.length; i++) {
      final String srcMask = srcMasks[i];
      if (null != srcMask && 0 < srcMask.trim().length()) {
        final Mask mask = new Mask(srcMask);
        final Source source = new Source(mask);
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
    final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    final List result = new ArrayList(providers.size());
    for (Iterator i = providers.keySet().iterator(); i.hasNext();) {
      result.add(String.valueOf(((Long) i.next()).longValue()));
    }
    return (String[]) result.toArray(new String[0]);
  }

  public String[] getProviders()
  {
    final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    final List result = new ArrayList(providers.size());
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      result.add(((Provider) i.next()).getName());
    }
    return (String[]) result.toArray(new String[0]);
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    this.name = name;
  }

  public String[] getSrcMasks()
  {
    return srcMasks;
  }

  public void setSrcMasks(final String[] srcMasks)
  {
    this.srcMasks = srcMasks;
  }

  public String[] getSrcSubjs()
  {
    return srcSubjs;
  }

  public void setSrcSubjs(final String[] srcSubjs)
  {
    this.srcSubjs = srcSubjs;
  }

  public String[] getDstMasks()
  {
    return dstMasks;
  }

  public void setDstMasks(final String[] dstMasks)
  {
    this.dstMasks = dstMasks;
  }

  public String[] getDstSubjs()
  {
    return dstSubjs;
  }

  public void setDstSubjs(final String[] dstSubjs)
  {
    this.dstSubjs = dstSubjs;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(final int priority)
  {
    this.priority = priority;
  }

  public boolean isEnabling()
  {
    return enabling;
  }

  public void setEnabling(final boolean enabling)
  {
    this.enabling = enabling;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(final boolean archiving)
  {
    this.archiving = archiving;
  }

  public boolean isBilling()
  {
    return billing;
  }

  public void setBilling(final boolean billing)
  {
    this.billing = billing;
  }

  public int getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(final int serviceId)
  {
    this.serviceId = serviceId;
  }

  public boolean isSuppressDeliveryReports()
  {
    return suppressDeliveryReports;
  }

  public void setSuppressDeliveryReports(final boolean suppressDeliveryReports)
  {
    this.suppressDeliveryReports = suppressDeliveryReports;
  }

  public boolean isActive()
  {
    return active;
  }

  public void setActive(final boolean active)
  {
    this.active = active;
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(final String srcSmeId)
  {
    this.srcSmeId = srcSmeId;
  }

  public String getDeliveryMode()
  {
    return deliveryMode;
  }

  public void setDeliveryMode(final String deliveryMode)
  {
    this.deliveryMode = deliveryMode;
  }

  public String getForwardTo()
  {
    return forwardTo;
  }

  public void setForwardTo(final String forwardTo)
  {
    this.forwardTo = forwardTo;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(final boolean hide)
  {
    this.hide = hide;
  }

  public boolean isForceReplayPath()
  {
    return forceReplayPath;
  }

  public void setForceReplayPath(final boolean forceReplayPath)
  {
    this.forceReplayPath = forceReplayPath;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(final String notes)
  {
    this.notes = notes;
  }

  public String getProvider()
  {
    return provider;
  }

  public void setProvider(final String provider)
  {
    this.provider = provider;
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(final long providerId)
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
    final Map result = new TreeMap();
    for (Iterator i = appContext.getGwRoutingManager().getSubjects().values().iterator(); i.hasNext();) {
      final Subject subject = (Subject) i.next();
      result.put(subject.getName(), subject.getDefaultSme().getId());
    }
    return result;
  }

  public Map getDstSubjPairs()
  {
    final GwRoute route = (GwRoute) appContext.getGwRoutingManager().getRoutes().get(getEditId());
    if (null != route) {
      final Map result = new TreeMap();
      for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
        final Destination destination = (Destination) i.next();
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
    if (null != route) {
      final Map result = new TreeMap();
      for (Iterator i = route.getDestinations().values().iterator(); i.hasNext();) {
        final Destination destination = (Destination) i.next();
        if (!destination.isSubject())
          result.put(destination.getName(), destination.getSme().getId());
      }
      return result;
    }
    return null;
  }

  public String getNew_dstMask()
  {
    return new_dstMask;
  }

  public void setNew_dstMask(String new_dstMask)
  {
    this.new_dstMask = new_dstMask;
  }

  public String getNew_dst_mask_sme_()
  {
    return new_dst_mask_sme_;
  }

  public void setNew_dst_mask_sme_(String new_dst_mask_sme_)
  {
    this.new_dst_mask_sme_ = new_dst_mask_sme_;
  }

  public boolean isTrafficRules_allowReceive()
  {
    return trafficRules_allowReceive;
  }

  public void setTrafficRules_allowReceive(final boolean trafficRules_allowReceive)
  {
    this.trafficRules_allowReceive = trafficRules_allowReceive;
  }

  public boolean isTrafficRules_allowAnswer()
  {
    return trafficRules_allowAnswer;
  }

  public void setTrafficRules_allowAnswer(final boolean trafficRules_allowAnswer)
  {
    this.trafficRules_allowAnswer = trafficRules_allowAnswer;
  }

  public String getTrafficRules_sendLimit()
  {
    return trafficRules_sendLimit;
  }

  public void setTrafficRules_sendLimit(final String trafficRules_sendLimit)
  {
    this.trafficRules_sendLimit = trafficRules_sendLimit;
  }

  public boolean isTrafficRules_allowPssrResp()
  {
    return trafficRules_allowPssrResp;
  }

  public void setTrafficRules_allowPssrResp(final boolean trafficRules_allowPssrResp)
  {
    this.trafficRules_allowPssrResp = trafficRules_allowPssrResp;
  }

  public boolean isTrafficRules_allowUssrRequest()
  {
    return trafficRules_allowUssrRequest;
  }

  public void setTrafficRules_allowUssrRequest(final boolean trafficRules_allowUssrRequest)
  {
    this.trafficRules_allowUssrRequest = trafficRules_allowUssrRequest;
  }

  public String getTrafficRules_ussdMiDialogLimit()
  {
    return trafficRules_ussdMiDialogLimit;
  }

  public void setTrafficRules_ussdMiDialogLimit(final String trafficRules_ussdMiDialogLimit)
  {
    this.trafficRules_ussdMiDialogLimit = trafficRules_ussdMiDialogLimit;
  }

  public boolean isTrafficRules_allowUssdDialogInit()
  {
    return trafficRules_allowUssdDialogInit;
  }

  public void setTrafficRules_allowUssdDialogInit(final boolean trafficRules_allowUssdDialogInit)
  {
    this.trafficRules_allowUssdDialogInit = trafficRules_allowUssdDialogInit;
  }

  public String getTrafficRules_ussdSiDialogLimit()
  {
    return trafficRules_ussdSiDialogLimit;
  }

  public void setTrafficRules_ussdSiDialogLimit(final String trafficRules_ussdSiDialogLimit)
  {
    this.trafficRules_ussdSiDialogLimit = trafficRules_ussdSiDialogLimit;
  }
}
