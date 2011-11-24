package ru.novosoft.smsc.admin.mcisme;

import org.junit.Test;
import ru.novosoft.smsc.admin.mcisme.Schedules.Counter;
import ru.novosoft.smsc.admin.mcisme.protocol.*;
import ru.novosoft.smsc.util.Address;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static ru.novosoft.smsc.admin.mcisme.MCIConverter.convert;

/**
 * author: Aleksandr Khalitov
 */
public class MCIConverterTest {

  private static final byte ALL_FLAGS_ENABLED_VAL = (byte) ((0x01) + (0x02) + (0x04) + (0x08) + (0x10));

  private static GetStatsResp createGetStatsResp() {
    GetStatsResp r = new GetStatsResp();
    r.setDelivered(1);
    r.setFailed(2);
    r.setMissed(3);
    r.setNotified(4);
    return r;
  }

  private static GetRunStatsResp createGetRunStatsResp() {
    GetRunStatsResp r = new GetRunStatsResp();
    r.setActiveTasks(1);
    r.setInQueueSize(2);
    r.setInSpeed(3);
    r.setOutQueueSize(4);
    r.setOutSpeed(5);
    return r;
  }

  private static GetSchedListResp createGetSchedListResp() {
    GetSchedListResp r = new GetSchedListResp();
    r.setTimeLines(createTimelines());
    return r;
  }

  private static TimeLineInfo[] createTimelines() {
    TimeLineInfo[] tI = new TimeLineInfo[10];
    for(int i=0;i<tI.length;i++) {
      TimeLineInfo timeLine = new TimeLineInfo();
      timeLine.setCount(10+i);
      timeLine.setTime(23213123312l+(i*1000));
      tI[i] = timeLine;
    }
    return tI;
  }
  private static void assertCounters(Iterable<Counter> counters) {
    for(Counter c : counters) {
      int i = c.getCount() - 10;
      assertEquals(23213123312l+(i*1000), c.getTime());
    }
  }

  private static GetSchedResp createGetSchedResp() {
    GetSchedResp r = new GetSchedResp();
    r.setEventsCount((byte) 7);
    r.setLastError(3123);
    r.setSubscriber("+79139489906");
    r.setTime(43214124124124l);
    return r;
  }


  private static Profile createProfile() {
    Profile p = new Profile();
    p.setNotify(true);
    p.setInform(false);
    p.setSubscriber(new Address("4324141241"));
    p.setWantNotifyMe(false);
    p.setAbsent(true);
    p.setBusy(true);
    p.setNoReplay(true);
    p.setUnconditional(true);
    p.setDetach(true);
    p.setInformTemplateId((byte) 3);
    p.setNotifyTemplateId((byte) 4);
    return p;
  }

  private static GetProfileResp createGetProfileResp() {
    GetProfileResp p = new GetProfileResp();
    p.setNotify(true);
    p.setInform(false);
    p.setWantNotifyMe(false);
    p.setEventMask(ALL_FLAGS_ENABLED_VAL);
    p.setInformTemplateId((byte) 3);
    p.setNotifyTemplateId((byte) 4);
    return p;
  }

  @Test
  public void testConvertProfile() {
    Profile p = createProfile();
    SetProfile setProfile = convert(p);
    assertEquals(setProfile.getSubscriber(), p.getSubscriber().getSimpleAddress());
    assertEquals(setProfile.getEventMask(), ALL_FLAGS_ENABLED_VAL);
    assertEquals(setProfile.getInformTemplateId(), p.getInformTemplateId());
    assertEquals(setProfile.getNotify(), p.isNotify());
    assertEquals(setProfile.getWantNotifyMe(), p.isWantNotifyMe());
    assertEquals(setProfile.getNotifyTemplateId(), p.getNotifyTemplateId());
    assertEquals(setProfile.getInform(), p.isInform());
  }

  @Test
  public void testConvertGetrofile() {
    GetProfileResp p = createGetProfileResp();
    Profile profile = convert(new Address("+79139489906"), p);
    assertEquals(profile.getSubscriber().getSimpleAddress(), "+79139489906");
    assertTrue(profile.isAbsent());
    assertTrue(profile.isBusy());
    assertTrue(profile.isNoReplay());
    assertTrue(profile.isUnconditional());
    assertTrue(profile.isDetach());
    assertEquals(profile.getInformTemplateId(), p.getInformTemplateId());
    assertEquals(profile.isNotify(), p.getNotify());
    assertEquals(profile.isWantNotifyMe(), p.getWantNotifyMe());
    assertEquals(profile.getNotifyTemplateId(), p.getNotifyTemplateId());
    assertEquals(profile.isInform(), p.getInform());
  }

  @Test
  public void testConvertShedule() {
    GetSchedResp gSR = createGetSchedResp();
    Schedule s = convert(gSR);
    assertEquals(gSR.getSubscriber(), s.getSubscriber().getSimpleAddress());
    assertEquals(gSR.getEventsCount(), s.getEventsCount());
    assertEquals(gSR.getLastError(), s.getLastError());
    assertEquals(gSR.getTime(), s.getTime());
  }

  @Test
  public void testConvertSchedList() {
    GetSchedListResp gSL = createGetSchedListResp();
    Schedules ss = convert(gSL);
    assertEquals(ss.getCounters().size(), 10);
    assertCounters(ss.getCounters());
  }

  @Test
  public void testConvertStats() {
    GetStatsResp gSR = createGetStatsResp();
    Statistics s = convert(gSR);
    assertEquals(gSR.getDelivered(), s.getDelivered());
    assertEquals(gSR.getFailed(), s.getFailed());
    assertEquals(gSR.getMissed(), s.getMissed());
    assertEquals(gSR.getNotified(), s.getNotified());
  }

  @Test
  public void testConvertRunStats() {
    GetRunStatsResp gSR = createGetRunStatsResp();
    RunStatistics s = convert(gSR);
    assertEquals(gSR.getActiveTasks(), s.getActiveTasks());
    assertEquals(gSR.getInQueueSize(), s.getInQueueSize());
    assertEquals(gSR.getInSpeed(), s.getInSpeed());
    assertEquals(gSR.getOutQueueSize(), s.getOutQueueSize());
    assertEquals(gSR.getOutSpeed(), s.getOutSpeed());
  }


}
