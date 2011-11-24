package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.mcisme.Schedules.Counter;
import ru.novosoft.smsc.admin.mcisme.protocol.*;
import ru.novosoft.smsc.util.Address;

/**
 * author: Aleksandr Khalitov
 */
public class MCIConverter {

  private static final int ABSENT_FLAG = 0x01;
  private static final int BUSY_FLAG = 0x02;
  private static final int NOREPLAY_FLAG = 0x04;
  private static final int UNCONDITIONAL_FLAG = 0x08;
  private static final int DETACH_FLAG = 0x10;

  private MCIConverter() {
  }

  public static Profile convert(Address subscriber, GetProfileResp r) {
    Profile p = new Profile();
    p.setSubscriber(subscriber);
    if(r.hasEventMask()) {
      int mask = r.getEventMask();
      p.setAbsent((mask& ABSENT_FLAG) == ABSENT_FLAG);
      p.setBusy((mask& BUSY_FLAG) == BUSY_FLAG);
      p.setNoReplay((mask& NOREPLAY_FLAG) == NOREPLAY_FLAG);
      p.setUnconditional((mask& UNCONDITIONAL_FLAG) == UNCONDITIONAL_FLAG);
      p.setDetach((mask& DETACH_FLAG) == DETACH_FLAG);
    }
    if(r.hasInform()) {
      p.setInform(r.getInform());
    }
    if(r.hasInformTemplateId()) {
      p.setInformTemplateId(r.getInformTemplateId());
    }
    if(r.hasNotifyTemplateId()) {
      p.setNotifyTemplateId(r.getNotifyTemplateId());
    }
    if(r.hasNotify()) {
      p.setNotify(r.getNotify());
    }
    if(r.hasWantNotifyMe()) {
      p.setWantNotifyMe(r.getWantNotifyMe());
    }
    return p;
  }

  public static SetProfile convert(Profile profile) {
    SetProfile s = new SetProfile();
    int eventMask = 0;
    if (profile.isAbsent()) eventMask += ABSENT_FLAG;
    if (profile.isBusy()) eventMask += BUSY_FLAG;
    if (profile.isNoReplay()) eventMask += NOREPLAY_FLAG;
    if (profile.isUnconditional()) eventMask += UNCONDITIONAL_FLAG;
    if (profile.isDetach()) eventMask += DETACH_FLAG;
    s.setEventMask((byte)eventMask);
    s.setInform(profile.isInform());
    s.setInformTemplateId(profile.getInformTemplateId());
    s.setNotify(profile.isNotify());
    s.setNotifyTemplateId(profile.getNotifyTemplateId());
    if(profile.getSubscriber() != null) {
      s.setSubscriber(profile.getSubscriber().getSimpleAddress());
    }
    s.setWantNotifyMe(profile.isWantNotifyMe());
    return s;
  }

  public static Schedule convert(GetSchedResp r) {
    Schedule s = new Schedule();
    if(r.hasEventsCount()) {
      s.setEventsCount(r.getEventsCount());
    }
    if(r.hasLastError()) {
      s.setLastError(r.getLastError());
    }
    if(r.hasTime()) {
      s.setTime(r.getTime());
    }
    if(r.hasSubscriber()) {
      s.setSubscriber(new Address(r.getSubscriber()));
    }
    return s;
  }

  public static Schedules convert(GetSchedListResp r) {
    Schedules s = new Schedules();
    if(r.hasTimeLines()) {
      for(TimeLineInfo t : r.getTimeLines()) {
        long time = t.hasTime() ? t.getTime() : 0;
        int count = t.hasCount() ? t.getCount() : 0;
        s.addCounter(new Counter(time, count));
      }
    }
    return s;
  }

  public static Statistics convert(GetStatsResp r) {
    Statistics s = new Statistics();
    if(r.hasDelivered()) {
      s.setDelivered(r.getDelivered());
    }
    if(r.hasFailed()) {
      s.setFailed(r.getFailed());
    }
    if(r.hasMissed()) {
      s.setMissed(r.getMissed());
    }
    if(r.hasNotified()) {
      s.setNotified(r.getNotified());
    }
    return s;
  }

  public static RunStatistics convert(GetRunStatsResp r) {
    RunStatistics s = new RunStatistics();
    if(r.hasActiveTasks()) {
      s.setActiveTasks(r.getActiveTasks());
    }
    if(r.hasInQueueSize()) {
      s.setInQueueSize(r.getInQueueSize());
    }
    if(r.hasInSpeed()) {
      s.setInSpeed(r.getInSpeed());
    }
    if(r.hasOutQueueSize()) {
      s.setOutQueueSize(r.getOutQueueSize());
    }
    if(r.hasOutSpeed()) {
      s.setOutSpeed(r.getOutSpeed());
    }
    return s;
  }
}
