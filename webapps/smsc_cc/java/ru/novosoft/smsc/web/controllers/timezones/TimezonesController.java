package ru.novosoft.smsc.web.controllers.timezones;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.timezone.Timezone;
import ru.novosoft.smsc.admin.timezone.TimezoneSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 07.10.2010
 * Time: 16:36:16
 */
public class TimezonesController extends SettingsMController<TimezoneSettings> {


  boolean initError = false;
  private TimezoneSettings settings;

  DynamicTableModel masksModel;
  DynamicTableModel subjsModel;
  private List timesonesItems;

  public TimezonesController() {
    super(WebContext.getInstance().getTimezoneManager());
    try {
      super.init();
      initModels();
    }
    catch (AdminException e) {
      addError(e);
      initError=true;
    }
  }

  private void initModels() {
    settings = getSettings();
    masksModel = new DynamicTableModel();
    subjsModel = new DynamicTableModel();
    for(Timezone tz : settings.getTimezones()) {
      DynamicTableRow row = new DynamicTableRow();

      if(tz.getMask()!=null) {
        row.setValue("mask",  tz.getMask().getSimpleAddress());
        row.setValue("tzm",tz.getTimezone().getID());
        masksModel.addRow(row);
      }
      else if(tz.getSubject()!=null) {
        row.setValue("subj",  tz.getSubject());
        row.setValue("tzs",tz.getTimezone().getID());
        subjsModel.addRow(row);
      }
    }
  }


  public TimeZone getDefaultTimeZone() {
    return settings.getDefaultTimeZone();
  }

  public void setDefaultTimeZone(TimeZone tz) throws AdminException {
    settings.setDefaultTimeZone(tz);
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public DynamicTableModel getMasksModel() {
    return masksModel;
  }

  public void setMasksModel(DynamicTableModel masksModel) {
    this.masksModel = masksModel;
  }

  public DynamicTableModel getSubjsModel() {
    return subjsModel;
  }

  public void setSubjsModel(DynamicTableModel subjsModel) {
    this.subjsModel = subjsModel;
  }

  public List<String> getTimesonesItems() {
    String[] tzIds = TimeZone.getAvailableIDs();
    Arrays.sort(tzIds);
    return Arrays.asList(tzIds);
  }

  public List<String> getAvailableSubjects() throws AdminException {
    List<String> ret = new ArrayList<String>();
    for(Subject s : WebContext.getInstance().getRouteSubjectManager().getSettings().getSubjects()) {
      ret.add(s.getName());
    }
    Collections.sort(ret);
    return ret;
  }

  public String reset() {
    try {
      resetSettings();
      initModels();
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String save() {
    try {

      List<Timezone> newTimeZones = new ArrayList<Timezone>();
      for(DynamicTableRow row : masksModel.getRows()) {
        String mask = (String) row.getValue("mask");
        try {
          Address a = new Address(mask);
          TimeZone tz = TimeZone.getTimeZone((String) row.getValue("tzm"));
          Timezone tzm = new Timezone(a,tz);
          newTimeZones.add(tzm);
        }
        catch (IllegalArgumentException e) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"timezones.illegal.mask",mask);
          return null;
        }
      }

      for(DynamicTableRow row : subjsModel.getRows()) {
        String subj = (String) row.getValue("subj");
        TimeZone tz = TimeZone.getTimeZone((String) row.getValue("tzs"));
        Timezone tzm = new Timezone(subj,tz);
        newTimeZones.add(tzm);
      }
      settings.setTimezones(newTimeZones);
      setSettings(settings);
      submitSettings();
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

}
