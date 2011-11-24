package ru.novosoft.smsc.admin.mcisme;

import mobi.eyeline.util.Time;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings.InformTemplate;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings.NotifyTemplate;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings.Rule;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings.ScheduleError;

import static org.junit.Assert.fail;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeSettingsTest {

  private static MCISmeSettings createSettings() {
    MCISmeSettings settings = new MCISmeSettings();
    settings.setCauseAbsent(1);
    settings.setCauseDetach(1);
    settings.setCauseBusy(1);
    settings.setCauseNoReply(1);
    settings.setCauseOther(1);
    settings.setCauseUnconditional(1);
    return settings;
  }


  private static InformTemplate createInformTemplate(int id, String name) {
    InformTemplate t = new InformTemplate();
    t.setGroup(true);
    t.setId(id);
    t.setMessage("hjgjhg");
    t.setMultiRow("multi row");
    t.setSingleRow("single row");
    t.setName(name);
    t.setUnknownCaller("unknown");
    return t;
  }

  private static NotifyTemplate createNotifyTemplate(int id, String name) {
    NotifyTemplate t = new NotifyTemplate();
    t.setId(id);
    t.setName(name);
    t.setMessage("dsadadasdasddasdas");
    return t;
  }



  private static Rule createRule(String name) {
    Rule r = new Rule();
    r.setName(name);
    r.setCause(1024);
    r.setInform(2);
    r.setPriority(1);
    r.setRegexp("dsadsadasdas");
    return r;
  }


  private static ScheduleError createScheduleError(String error) {
    ScheduleError r = new ScheduleError();
    r.setError(error);
    r.setPeriod(new Time(1,1,1));
    return r;
  }


  @Test
  public void validateRuleOk() throws AdminException {
    Rule r1 = createRule("name1");
    MCISmeSettings settings = createSettings();
    Rule r2 = createRule("name2");
    settings.addRule(r1);
    settings.validate();
    settings.addRule(r2);
    settings.validate();
  }

  @Test
  public void validateRuleIntersection() throws AdminException {
    Rule r1 = createRule("name1");
    MCISmeSettings settings = createSettings();
    Rule r2 = createRule("name1");
    settings.addRule(r1);
    settings.validate();
    settings.addRule(r2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }


  @Test
  public void validateErrorOk() throws AdminException {
    ScheduleError r1 = createScheduleError("1179");
    MCISmeSettings settings = createSettings();
    ScheduleError r2 = createScheduleError("1024");
    settings.addScheduleError(r1);
    settings.validate();
    settings.addScheduleError(r2);
    settings.validate();
  }

  @Test
  public void validateErrorIntersection() throws AdminException {
    ScheduleError r1 = createScheduleError("1179");
    MCISmeSettings settings = createSettings();
    ScheduleError r2 = createScheduleError("1179");
    settings.addScheduleError(r1);
    settings.validate();
    settings.addScheduleError(r2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }

  @Test
  public void validateNotifyOk() throws AdminException {
    NotifyTemplate t1 = createNotifyTemplate(0, "name1");
    NotifyTemplate t2 = createNotifyTemplate(1, "name2");
    MCISmeSettings settings = createSettings();
    settings.addNotifyTemplate(t1);
    settings.addNotifyTemplate(t2);
    settings.validate();
  }


  @Test
  public void validateNotifyIdIntersection() throws AdminException {
    NotifyTemplate t1 = createNotifyTemplate(0, "name1");
    NotifyTemplate t2 = createNotifyTemplate(0, "name2");
    MCISmeSettings settings = createSettings();
    settings.addNotifyTemplate(t1);
    settings.validate();
    settings.addNotifyTemplate(t2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }
  @Test
  public void validateNotifyNameIntersection() throws AdminException {
    NotifyTemplate t1 = createNotifyTemplate(0, "name1");
    NotifyTemplate t2 = createNotifyTemplate(1, "name1");
    MCISmeSettings settings = createSettings();
    settings.addNotifyTemplate(t1);
    settings.validate();
    settings.addNotifyTemplate(t2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }

  @Test
  public void validateInformOk() throws AdminException {
    InformTemplate t1 = createInformTemplate(0, "name1");
    InformTemplate t2 = createInformTemplate(1, "name2");
    MCISmeSettings settings = createSettings();
    settings.addInformTemplate(t1);
    settings.addInformTemplate(t2);
    settings.validate();
  }

  @Test
  public void validateInformIdIntersection() throws AdminException {
    InformTemplate t1 = createInformTemplate(0, "name1");
    InformTemplate t2 = createInformTemplate(0, "name2");
    MCISmeSettings settings = createSettings();
    settings.addInformTemplate(t1);
    settings.validate();
    settings.addInformTemplate(t2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }
  @Test
  public void validateInformNameIntersection() throws AdminException {
    InformTemplate t1 = createInformTemplate(0, "name1");
    InformTemplate t2 = createInformTemplate(1, "name1");
    MCISmeSettings settings = createSettings();
    settings.addInformTemplate(t1);
    settings.validate();
    settings.addInformTemplate(t2);
    try{
      settings.validate();
      fail();
    }catch (AdminException ignored){}
  }
}
