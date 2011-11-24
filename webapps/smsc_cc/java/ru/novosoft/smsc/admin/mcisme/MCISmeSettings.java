package ru.novosoft.smsc.admin.mcisme;

import mobi.eyeline.util.Time;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * author: Aleksandr Khalitov
 */
@SuppressWarnings({"OverlyComplexClass", "ClassWithTooManyMethods"})
public class MCISmeSettings {

  private static final ValidationHelper vh = new ValidationHelper(MCISmeSettings.class);

  private Address address;
  private String svcType;
  private int protocolId;

  private int daysValid;

  private String callingMask;
  private String calledMask;

  private int maxDataSmRegistrySize;
  private int pduDispatchersCount;

  private int unrespondedMessagesMax;
  private int unrespondedMessagesSleep;

  private int outgoingSpeedMax;
  private String responceWaitTime;
  private int inputQueueSize;

  private boolean forceInform;
  private boolean forceNotify;
  private boolean defaultInform;
  private boolean defaultNotify;
  private boolean defaultWantNotifyMe;
  private int defaultReasonsMask;
  private boolean useWantNotifyPolicy;
  private boolean groupSmsByCallingAbonent;

  private String adminHost;
  private int adminPort;

  private String smscHost;
  private int smscPort;

  private String smscSid;
  private int smscTimeout;

  private String smscPassword;

  private int releaseStrategy;

  private boolean skipUnknownCaller;

  private int causeDetach;

  private boolean informDetach;

  private int causeBusy;
  private boolean informBusy;
  private int causeNoReply;

  private boolean informNoReply;
  private int causeUnconditional;
  private boolean informUnconditional;
  private int causeAbsent;
  private boolean informAbsent;
  private int causeOther;
  private boolean informOther;

  private Address redirectionAddress;
  private String countryCode;
  private String timeZoneFileLocation;
  private String routesFileLocation;
  private String eventStorageLocation;
  private String eventLifeTime;

  private String eventPolicyRegistration;
  private int maxEvents;
  private String advertServer;
  private int advertPort;
  private int advertTimeout;
  private boolean useAdvert;

  private String profStorageLocation;
  private int profStoragePort;
  private String profStorageHost;

  private Time resendingPeriod;
  private String schedOnBusy;
  private String schedDelay;

  private String statDir;

  private final Collection<ScheduleError> scheduleErrors = new LinkedList<ScheduleError>();

  private final Collection<Circuit> circuits = new LinkedList<Circuit>();

  private final Collection<Rule> rules = new LinkedList<Rule>();

  private final Collection<InformTemplate> informTemplates = new LinkedList<InformTemplate>();

  private final Collection<NotifyTemplate> notifyTemplates = new LinkedList<NotifyTemplate>();

  private int defaultInformTemplate;

  private int defaultNotifyTemplate;

  public MCISmeSettings() {
  }

  @SuppressWarnings({"OverlyLongMethod"})
  public MCISmeSettings(MCISmeSettings s) {
    this.address = s.address == null ? null : new Address(s.address);
    this.svcType = s.svcType;
    this.protocolId = s.protocolId;
    this.daysValid = s.daysValid;
    this.callingMask = s.callingMask;
    this.calledMask = s.calledMask;
    this.maxDataSmRegistrySize = s.maxDataSmRegistrySize;
    this.pduDispatchersCount = s.pduDispatchersCount;
    this.unrespondedMessagesMax = s.unrespondedMessagesMax;
    this.unrespondedMessagesSleep = s.unrespondedMessagesSleep;
    this.outgoingSpeedMax = s.outgoingSpeedMax;
    this.responceWaitTime = s.responceWaitTime;
    this.inputQueueSize = s.inputQueueSize;
    this.forceInform = s.forceInform;
    this.forceNotify = s.forceNotify;
    this.defaultInform = s.defaultInform;
    this.defaultNotify = s.defaultNotify;
    this.defaultWantNotifyMe = s.defaultWantNotifyMe;
    this.defaultReasonsMask = s.defaultReasonsMask;
    this.useWantNotifyPolicy = s.useWantNotifyPolicy;
    this.groupSmsByCallingAbonent = s.groupSmsByCallingAbonent;
    this.adminHost = s.adminHost;
    this.adminPort = s.adminPort;
    this.smscHost = s.smscHost;
    this.smscPort = s.smscPort;
    this.smscSid = s.smscSid;
    this.smscTimeout = s.smscTimeout;
    this.smscPassword = s.smscPassword;
    this.releaseStrategy = s.releaseStrategy;
    this.skipUnknownCaller = s.skipUnknownCaller;
    this.causeDetach = s.causeDetach;
    this.informDetach = s.informDetach;
    this.causeBusy = s.causeBusy;
    this.informBusy = s.informBusy;
    this.causeNoReply = s.causeNoReply;
    this.informNoReply = s.informNoReply;
    this.causeUnconditional = s.causeUnconditional;
    this.informUnconditional = s.informUnconditional;
    this.causeAbsent = s.causeAbsent;
    this.informAbsent = s.informAbsent;
    this.causeOther = s.causeOther;
    this.informOther = s.informOther;
    this.redirectionAddress = s.redirectionAddress == null ? null : new Address(s.redirectionAddress);
    this.countryCode = s.countryCode;
    this.timeZoneFileLocation = s.timeZoneFileLocation;
    this.routesFileLocation = s.routesFileLocation;
    this.eventStorageLocation = s.eventStorageLocation;
    this.eventLifeTime = s.eventLifeTime;
    this.eventPolicyRegistration = s.eventPolicyRegistration;
    this.maxEvents = s.maxEvents;
    this.advertServer = s.advertServer;
    this.advertPort = s.advertPort;
    this.advertTimeout = s.advertTimeout;
    this.useAdvert = s.useAdvert;
    this.profStorageLocation = s.profStorageLocation;
    this.profStoragePort = s.profStoragePort;
    this.profStorageHost = s.profStorageHost;
    this.resendingPeriod = s.resendingPeriod;
    this.schedOnBusy = s.schedOnBusy;
    this.schedDelay = s.schedDelay;
    this.statDir = s.statDir;
    initRules(s);
    initCircuits(s);
    initScheduleErrors(s);
    initInformTempates(s);
    initNotifyTemplates(s);
  }



  private void initRules(MCISmeSettings s) {
    for(Rule r : s.rules) {
      this.rules.add(new Rule(r));
    }
  }

  private void initCircuits(MCISmeSettings s) {
    for(Circuit c : s.circuits) {
      this.circuits.add(new Circuit(c));
    }
  }

  private void initScheduleErrors(MCISmeSettings s) {
    for(ScheduleError e : s.scheduleErrors) {
      this.scheduleErrors.add(new ScheduleError(e));
    }
  }

  private void initInformTempates(MCISmeSettings s) {
    for(InformTemplate t : s.informTemplates) {
      this.informTemplates.add(new InformTemplate(t));
    }
    defaultInformTemplate = s.defaultInformTemplate;
  }

  private void initNotifyTemplates(MCISmeSettings s) {
    for(NotifyTemplate t : s.notifyTemplates) {
      this.notifyTemplates.add(new NotifyTemplate(t));
    }
    defaultNotifyTemplate = s.defaultNotifyTemplate;
  }


  private static final Integer[] RELEASE_CAUSES = new Integer[]{1,2,3,4,5,8,9,16,17,18,19,20,21,22,27,28,29,31,34,38,41,42,43,44,46,47,
      50,53,55,57,58,62,63,65,69,70,79,83,84,87,88,90,91,95,97,99,102,103,110,111,121,127};

  public void validate() throws AdminException{


    vh.checkIn("causeAbsent", causeAbsent, RELEASE_CAUSES);
    vh.checkIn("causeBusy", causeBusy, RELEASE_CAUSES);
    vh.checkIn("causeDetach", causeDetach, RELEASE_CAUSES);
    vh.checkIn("causeNoReply", causeNoReply, RELEASE_CAUSES);
    vh.checkIn("causeOther", causeOther, RELEASE_CAUSES);
    vh.checkIn("causeUnconditional", causeUnconditional, RELEASE_CAUSES);

    validateCircuits();
    validateRules();
    validateSchedulesErrors();
    validateInformTemplates();
    validateNotifyTemplates();

  }

  private void validateCircuits() throws AdminException{
    for(Circuit c1 : circuits) {
      c1.validate();
      for(Circuit c2 : circuits) {
        if(c1 != c2) {
          if(c1.getMscId().equals(c2.getMscId())) {
            throw new MCISmeException("circuits_mscid_intersection");
          }
        }
      }
    }
  }

  private void validateSchedulesErrors() throws AdminException{
    for(ScheduleError e1 : scheduleErrors) {
      for(ScheduleError e2 : scheduleErrors) {
        if(e1 != e2) {
          if(e1.error.equals(e2.error)) {
            throw new MCISmeException("schedules_errors_intersection");
          }
        }
      }
    }
  }

  private void validateRules() throws AdminException {
    for(Rule r1 : rules) {
      r1.validate();
      for(Rule r2 : rules) {
        if(r1 != r2) {
          r2.validate();
          if(r1.getName().equals(r2.getName())) {
            throw new MCISmeException("rules_names_intersection");
          }
        }
      }
    }
  }

  private void validateNotifyTemplates() throws AdminException{
    if(notifyTemplates.isEmpty()) {
      return;
    }
    boolean foundDef = false;
    for(NotifyTemplate t1 : notifyTemplates) {
      t1.validate();
      vh.checkNotNull("template.id", t1.id);
      vh.checkNotNull("template.name", t1.name);
      for(NotifyTemplate t2 : notifyTemplates) {
        if(t1 != t2) {
          if(t1.id == t2.id) {
            throw new MCISmeException("templates_ids_intersection");
          }
          if(t1.name.equals(t2.name)) {
            throw new MCISmeException("templates_names_intersection");
          }
        }
      }
      if(t1.getId() == defaultInformTemplate) {
        foundDef = true;
      }
    }
    if(!foundDef) {
      throw new MCISmeException("templates_def_not_found");
    }
  }

  private void validateInformTemplates() throws AdminException{
    if(informTemplates.isEmpty()) {
      return;
    }
    boolean foundDef = false;
    for(InformTemplate t1 : informTemplates) {
      t1.validate();
      vh.checkNotNull("template.id", t1.id);
      vh.checkNotNull("template.name", t1.name);
      for(InformTemplate t2 : informTemplates) {
        if(t1 != t2) {
          if(t1.id == t2.id) {
            throw new MCISmeException("templates_ids_intersection");
          }
          if(t1.name.equals(t2.name)) {
            throw new MCISmeException("templates_names_intersection");
          }
        }
      }
      if(t1.getId() == defaultInformTemplate) {
        foundDef = true;
      }
    }
    if(!foundDef) {
      throw new MCISmeException("templates_def_not_found");
    }
  }

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  public int getProtocolId() {
    return protocolId;
  }

  public void setProtocolId(int protocolId) {
    this.protocolId = protocolId;
  }

  public int getDaysValid() {
    return daysValid;
  }

  public void setDaysValid(int daysValid) {
    this.daysValid = daysValid;
  }

  public String getCallingMask() {
    return callingMask;
  }

  public void setCallingMask(String callingMask) {
    this.callingMask = callingMask;
  }

  public String getCalledMask() {
    return calledMask;
  }

  public void setCalledMask(String calledMask) {
    this.calledMask = calledMask;
  }

  public int getMaxDataSmRegistrySize() {
    return maxDataSmRegistrySize;
  }

  public void setMaxDataSmRegistrySize(int maxDataSmRegistrySize) {
    this.maxDataSmRegistrySize = maxDataSmRegistrySize;
  }

  public int getPduDispatchersCount() {
    return pduDispatchersCount;
  }

  public void setPduDispatchersCount(int pduDispatchersCount) {
    this.pduDispatchersCount = pduDispatchersCount;
  }

  public int getUnrespondedMessagesMax() {
    return unrespondedMessagesMax;
  }

  public void setUnrespondedMessagesMax(int unrespondedMessagesMax) {
    this.unrespondedMessagesMax = unrespondedMessagesMax;
  }

  public int getUnrespondedMessagesSleep() {
    return unrespondedMessagesSleep;
  }

  public void setUnrespondedMessagesSleep(int unrespondedMessagesSleep) {
    this.unrespondedMessagesSleep = unrespondedMessagesSleep;
  }

  public int getOutgoingSpeedMax() {
    return outgoingSpeedMax;
  }

  public void setOutgoingSpeedMax(int outgoingSpeedMax) {
    this.outgoingSpeedMax = outgoingSpeedMax;
  }

  public String getResponceWaitTime() {
    return responceWaitTime;
  }

  public void setResponceWaitTime(String responceWaitTime) {
    this.responceWaitTime = responceWaitTime;
  }

  public int getInputQueueSize() {
    return inputQueueSize;
  }

  public void setInputQueueSize(int inputQueueSize) {
    this.inputQueueSize = inputQueueSize;
  }

  public boolean isForceInform() {
    return forceInform;
  }

  public void setForceInform(boolean forceInform) {
    this.forceInform = forceInform;
  }

  public boolean isForceNotify() {
    return forceNotify;
  }

  public void setForceNotify(boolean forceNotify) {
    this.forceNotify = forceNotify;
  }

  public boolean isDefaultInform() {
    return defaultInform;
  }

  public void setDefaultInform(boolean defaultInform) {
    this.defaultInform = defaultInform;
  }

  public boolean isDefaultNotify() {
    return defaultNotify;
  }

  public void setDefaultNotify(boolean defaultNotify) {
    this.defaultNotify = defaultNotify;
  }

  public boolean isDefaultWantNotifyMe() {
    return defaultWantNotifyMe;
  }

  public void setDefaultWantNotifyMe(boolean defaultWantNotifyMe) {
    this.defaultWantNotifyMe = defaultWantNotifyMe;
  }

  public int getDefaultReasonsMask() {
    return defaultReasonsMask;
  }

  public void setDefaultReasonsMask(int defaultReasonsMask) {
    this.defaultReasonsMask = defaultReasonsMask;
  }

  public boolean isUseWantNotifyPolicy() {
    return useWantNotifyPolicy;
  }

  public void setUseWantNotifyPolicy(boolean useWantNotifyPolicy) {
    this.useWantNotifyPolicy = useWantNotifyPolicy;
  }

  public boolean isGroupSmsByCallingAbonent() {
    return groupSmsByCallingAbonent;
  }

  public void setGroupSmsByCallingAbonent(boolean groupSmsByCallingAbonent) {
    this.groupSmsByCallingAbonent = groupSmsByCallingAbonent;
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String adminHost) {
    this.adminHost = adminHost;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) {
    this.adminPort = adminPort;
  }

  public String getSmscHost() {
    return smscHost;
  }

  public void setSmscHost(String smscHost) {
    this.smscHost = smscHost;
  }

  public int getSmscPort() {
    return smscPort;
  }

  public void setSmscPort(int smscPort) {
    this.smscPort = smscPort;
  }

  public String getSmscSid() {
    return smscSid;
  }

  public void setSmscSid(String smscSid) {
    this.smscSid = smscSid;
  }

  public int getSmscTimeout() {
    return smscTimeout;
  }

  public void setSmscTimeout(int smscTimeout) {
    this.smscTimeout = smscTimeout;
  }

  public String getSmscPassword() {
    return smscPassword;
  }

  public void setSmscPassword(String smscPassword) {
    this.smscPassword = smscPassword;
  }

  public int getReleaseStrategy() {
    return releaseStrategy;
  }

  public void setReleaseStrategy(int releaseStrategy) {
    this.releaseStrategy = releaseStrategy;
  }

  public boolean isSkipUnknownCaller() {
    return skipUnknownCaller;
  }

  public void setSkipUnknownCaller(boolean skipUnknownCaller) {
    this.skipUnknownCaller = skipUnknownCaller;
  }

  public int getCauseDetach() {
    return causeDetach;
  }

  public void setCauseDetach(int causeDetach) {
    this.causeDetach = causeDetach;
  }

  public boolean isInformDetach() {
    return informDetach;
  }

  public void setInformDetach(boolean informDetach) {
    this.informDetach = informDetach;
  }

  public int getCauseBusy() {
    return causeBusy;
  }

  public void setCauseBusy(int causeBusy) {
    this.causeBusy = causeBusy;
  }

  public boolean isInformBusy() {
    return informBusy;
  }

  public void setInformBusy(boolean informBusy) {
    this.informBusy = informBusy;
  }

  public int getCauseNoReply() {
    return causeNoReply;
  }

  public void setCauseNoReply(int causeNoReply) {
    this.causeNoReply = causeNoReply;
  }

  public boolean isInformNoReply() {
    return informNoReply;
  }

  public void setInformNoReply(boolean informNoReply) {
    this.informNoReply = informNoReply;
  }

  public int getCauseUnconditional() {
    return causeUnconditional;
  }

  public void setCauseUnconditional(int causeUnconditional) {
    this.causeUnconditional = causeUnconditional;
  }

  public boolean isInformUnconditional() {
    return informUnconditional;
  }

  public void setInformUnconditional(boolean informUnconditional) {
    this.informUnconditional = informUnconditional;
  }

  public int getCauseAbsent() {
    return causeAbsent;
  }

  public void setCauseAbsent(int causeAbsent) {
    this.causeAbsent = causeAbsent;
  }

  public boolean isInformAbsent() {
    return informAbsent;
  }

  public void setInformAbsent(boolean informAbsent) {
    this.informAbsent = informAbsent;
  }

  public int getCauseOther() {
    return causeOther;
  }

  public void setCauseOther(int causeOther) {
    this.causeOther = causeOther;
  }

  public boolean isInformOther() {
    return informOther;
  }

  public void setInformOther(boolean informOther) {
    this.informOther = informOther;
  }

  public Address getRedirectionAddress() {
    return redirectionAddress;
  }

  public void setRedirectionAddress(Address redirectionAddress) {
    this.redirectionAddress = redirectionAddress;
  }

  public String getCountryCode() {
    return countryCode;
  }

  public void setCountryCode(String countryCode) {
    this.countryCode = countryCode;
  }

  public String getTimeZoneFileLocation() {
    return timeZoneFileLocation;
  }

  public void setTimeZoneFileLocation(String timeZoneFileLocation) {
    this.timeZoneFileLocation = timeZoneFileLocation;
  }

  public String getRoutesFileLocation() {
    return routesFileLocation;
  }

  public void setRoutesFileLocation(String routesFileLocation) {
    this.routesFileLocation = routesFileLocation;
  }

  public String getEventStorageLocation() {
    return eventStorageLocation;
  }

  public void setEventStorageLocation(String eventStorageLocation) {
    this.eventStorageLocation = eventStorageLocation;
  }

  public String getEventLifeTime() {
    return eventLifeTime;
  }

  public void setEventLifeTime(String eventLifeTime) {
    this.eventLifeTime = eventLifeTime;
  }

  public String getEventPolicyRegistration() {
    return eventPolicyRegistration;
  }

  public void setEventPolicyRegistration(String eventPolicyRegistration) {
    this.eventPolicyRegistration = eventPolicyRegistration;
  }

  public int getMaxEvents() {
    return maxEvents;
  }

  public void setMaxEvents(int maxEvents) {
    this.maxEvents = maxEvents;
  }

  public String getAdvertServer() {
    return advertServer;
  }

  public void setAdvertServer(String advertServer) {
    this.advertServer = advertServer;
  }

  public int getAdvertPort() {
    return advertPort;
  }

  public void setAdvertPort(int advertPort) {
    this.advertPort = advertPort;
  }

  public int getAdvertTimeout() {
    return advertTimeout;
  }

  public void setAdvertTimeout(int advertTimeout) {
    this.advertTimeout = advertTimeout;
  }

  public boolean isUseAdvert() {
    return useAdvert;
  }

  public void setUseAdvert(boolean useAdvert) {
    this.useAdvert = useAdvert;
  }

  public String getProfStorageLocation() {
    return profStorageLocation;
  }

  public void setProfStorageLocation(String profStorageLocation) {
    this.profStorageLocation = profStorageLocation;
  }

  public int getProfStoragePort() {
    return profStoragePort;
  }

  public void setProfStoragePort(int profStoragePort) {
    this.profStoragePort = profStoragePort;
  }

  public String getProfStorageHost() {
    return profStorageHost;
  }

  public void setProfStorageHost(String profStorageHost) {
    this.profStorageHost = profStorageHost;
  }

  public Time getResendingPeriod() {
    return resendingPeriod;
  }

  public void setResendingPeriod(Time resendingPeriod) {
    this.resendingPeriod = resendingPeriod;
  }

  public String getSchedOnBusy() {
    return schedOnBusy;
  }

  public void setSchedOnBusy(String schedOnBusy) {
    this.schedOnBusy = schedOnBusy;
  }

  public String getSchedDelay() {
    return schedDelay;
  }

  public void setSchedDelay(String schedDelay) {
    this.schedDelay = schedDelay;
  }

  public String getStatDir() {
    return statDir;
  }

  public void setStatDir(String statDir) {
    this.statDir = statDir;
  }

  public int getDefaultInformTemplate() {
    return defaultInformTemplate;
  }

  public void setDefaultInformTemplate(int defaultInformTemplate) {
    this.defaultInformTemplate = defaultInformTemplate;
  }

  public int getDefaultNotifyTemplate() {
    return defaultNotifyTemplate;
  }

  public void setDefaultNotifyTemplate(int defaultNotifyTemplate) {
    this.defaultNotifyTemplate = defaultNotifyTemplate;
  }

  public Collection<ScheduleError> getScheduleErrors() {
    return scheduleErrors;
  }

  public Collection<Circuit> getCircuits() {
    return circuits;
  }

  public Collection<Rule> getRules() {
    return rules;
  }

  public Collection<InformTemplate> getInformTemplates() {
    return informTemplates;
  }

  public Collection<NotifyTemplate> getNotifyTemplates() {
    return notifyTemplates;
  }

  public void clearScheduleErrors() {
    scheduleErrors.clear();
  }

  public void clearCircuits() {
    circuits.clear();
  }

  public void clearRules() {
    rules.clear();
  }

  public synchronized void clearInformTemplates() {
    informTemplates.clear();
  }

  public synchronized void clearNotifyTemplates() {
    notifyTemplates.clear();
  }

  public void addScheduleError(ScheduleError e) {
    scheduleErrors.add(e);
  }

  public void addCircuit(Circuit e) {
    circuits.add(e);
  }

  public void addRule(Rule rule) {
    rules.add(rule);
  }

  public void removeRule(String name) {
    if(name != null) {
      Iterator<Rule> i = rules.iterator();
      while(i.hasNext()) {
        Rule r = i.next();
        if(name.equals(r.name)) {
          i.remove();
          break;
        }
      }
    }
  }

  public void addInformTemplate(InformTemplate template) {
    informTemplates.add(template);
  }

  public void addNotifyTemplate(NotifyTemplate template) {
    notifyTemplates.add(template);
  }

  public void removeNotifyTemplate(int id) {
    Iterator<NotifyTemplate> i = notifyTemplates.iterator();
    while(i.hasNext()) {
      if(id == i.next().id){
        i.remove();
      }
    }
  }

  public void removeInformTemplate(int id) {
    Iterator<InformTemplate> i = informTemplates.iterator();
    while(i.hasNext()) {
      if(id == i.next().id){
        i.remove();
      }
    }
  }

  public static class Circuit {
    private String mscId;
    private int hsn;
    private int spn;
    private String tsm;

    public Circuit() {
    }

    public void validate() throws AdminException {
      vh.checkNotNull("circuit.mscid", mscId);
      vh.checkNotNull("circuit.tsm", tsm);
    }

    public Circuit(Circuit c) {
      this.mscId = c.mscId;
      this.hsn = c.hsn;
      this.spn = c.spn;
      this.tsm = c.tsm;
    }

    public String getMscId() {
      return mscId;
    }

    public void setMscId(String mscId) {
      this.mscId = mscId;
    }

    public int getHsn() {
      return hsn;
    }

    public void setHsn(int hsn) {
      this.hsn = hsn;
    }

    public int getSpn() {
      return spn;
    }

    public void setSpn(int spn) {
      this.spn = spn;
    }

    public String getTsm() {
      return tsm;
    }

    public void setTsm(String tsm) {
      this.tsm = tsm;
    }
  }

  public static class Rule {
    private String name;
    private int cause;
    private int inform;
    private int priority;
    private String regexp;

    public Rule() {
    }

    public Rule(Rule r) {
      this.name = r.name;
      this.cause = r.cause;
      this.inform = r.inform;
      this.priority = r.priority;
      this.regexp = r.regexp;
    }

    public void validate() throws AdminException {
      vh.checkNotNull("rule.name", name);
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public int getCause() {
      return cause;
    }

    public void setCause(int cause) {
      this.cause = cause;
    }

    public int getInform() {
      return inform;
    }

    public void setInform(int inform) {
      this.inform = inform;
    }

    public int getPriority() {
      return priority;
    }

    public void setPriority(int priority) {
      this.priority = priority;
    }

    public String getRegexp() {
      return regexp;
    }

    public void setRegexp(String regexp) {
      this.regexp = regexp;
    }
  }

  public static class InformTemplate {

    private int id;

    private String name;

    private boolean group;

    private String message;

    private String multiRow;

    private String singleRow;

    private String unknownCaller;

    public InformTemplate() {
    }

    public InformTemplate(InformTemplate t) {
      this.id = t.id;
      this.name = t.name;
      this.group = t.group;
      this.message = t.message;
      this.multiRow = t.multiRow;
      this.singleRow = t.singleRow;
      this.unknownCaller = t.unknownCaller;
    }

    public void validate() throws AdminException {
      vh.checkNotNull("template.name", name);
    }

    public int getId() {
      return id;
    }

    public void setId(int id) {
      this.id = id;
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public boolean isGroup() {
      return group;
    }

    public void setGroup(boolean group) {
      this.group = group;
    }

    public String getMessage() {
      return message;
    }

    public void setMessage(String message) {
      this.message = message;
    }

    public String getMultiRow() {
      return multiRow;
    }

    public void setMultiRow(String multiRow) {
      this.multiRow = multiRow;
    }

    public String getSingleRow() {
      return singleRow;
    }

    public void setSingleRow(String singleRow) {
      this.singleRow = singleRow;
    }

    public String getUnknownCaller() {
      return unknownCaller;
    }

    public void setUnknownCaller(String unknownCaller) {
      this.unknownCaller = unknownCaller;
    }
  }

  public static class NotifyTemplate {

    private int id;

    private String name;

    private String message;

    public NotifyTemplate() {
    }


    public NotifyTemplate(NotifyTemplate t) {
      this.id = t.id;
      this.name = t.name;
      this.message = t.message;
    }

    public void validate() throws AdminException {
      vh.checkNotNull("template.name",name);
    }

    public int getId() {
      return id;
    }

    public void setId(int id) {
      this.id = id;
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public String getMessage() {
      return message;
    }

    public void setMessage(String message) {
      this.message = message;
    }
  }

  public static class ScheduleError {
    private String error;
    private Time period;

    public ScheduleError() {
    }

    public ScheduleError(ScheduleError s) {
      this.error = s.error;
      this.period = s.period == null ? null : new Time(s.period);
    }

    public void validate() throws AdminException {
      vh.checkNotNull("schedules.error", error);
      vh.checkNotNull("schedules.period", period);
    }

    public String getError() {
      return error;
    }

    public void setError(String error) {
      this.error = error;
    }

    public Time getPeriod() {
      return period;
    }

    public void setPeriod(Time period) {
      this.period = period;
    }
  }
}
