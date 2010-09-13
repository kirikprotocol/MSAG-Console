package ru.novosoft.smsc.admin.cluster_controller;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 * @author Artem Snopkov
 */
public class TestLoggersHelper {

  public Collection<CCLoggingInfo> loggers;

  public TestLoggersHelper() {
    loggers = new ArrayList<CCLoggingInfo>();
    Collections.addAll(loggers,
        new CCLoggingInfo("AgentLst", CCLoggingInfo.Level.WARN),
        new CCLoggingInfo("aliasman", null),
        new CCLoggingInfo("eventqueue", null),
        new CCLoggingInfo("inmancom", null),
        new CCLoggingInfo("maplimits", null),
        new CCLoggingInfo("pktwrt", null),
        new CCLoggingInfo("resched", null),
        new CCLoggingInfo("sched", null),
        new CCLoggingInfo("smeman", null),
        new CCLoggingInfo("dpf.cmpct", null),
        new CCLoggingInfo("dpf.track", CCLoggingInfo.Level.DEBUG),
        new CCLoggingInfo("dpf.track.fsd", null),
        new CCLoggingInfo("map.dialog", null),
        new CCLoggingInfo("map.msg", null),
        new CCLoggingInfo("map.proxy", null)
    );
  }

  public Collection<CCLoggingInfo> getLoggers() {
    return loggers;
  }

  public void setLoggers(Collection<CCLoggingInfo> loggers) {
    this.loggers = loggers;
  }
}
