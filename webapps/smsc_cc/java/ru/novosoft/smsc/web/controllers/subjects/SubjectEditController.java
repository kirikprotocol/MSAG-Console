package ru.novosoft.smsc.web.controllers.subjects;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.controllers.SettingsMController;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import javax.swing.table.DefaultTableModel;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 01.10.2010
 * Time: 14:56:14
 */
public class SubjectEditController extends SettingsMController<RouteSubjectSettings> {

  String oldName=null;
  private boolean initError;
  RouteSubjectSettings settings;
  Subject subj=null;
  private DynamicTableModel masksModel;
  private DynamicTableModel subjectsModel;


  public SubjectEditController() {
    super(WebContext.getInstance().getRouteSubjectManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
    }
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String getOldName() {
    return oldName;
  }

  public void setOldName(String oldName) {
    if(oldName!=null && oldName.trim().length()==0) this.oldName=null;
    else this.oldName = oldName;
  }

  @Override
  protected void init() throws AdminException {
     super.init();
     settings = getSettings();
     masksModel=new DynamicTableModel();
     subjectsModel=new DynamicTableModel();
     setOldName(getRequestParameter("oldName"));
     if(oldName!=null) {
       for( Subject subj : settings.getSubjects() ) {
         if(subj.getName().equals(oldName)) {
           this.subj = subj;
           for(Address mask : subj.getMasks()) {
             DynamicTableRow row = new DynamicTableRow();
             row.setValue("mask", mask.getSimpleAddress());
             masksModel.addRow(row);
           }
           for(String child : subj.getChildren()) {
             DynamicTableRow row = new DynamicTableRow();
             row.setValue("child", child);
             masksModel.addRow(row);
           }
           break;
         }
       }

     }
     else {
       subj = new Subject();
     }
     if(subj==null) {
       addLocalizedMessage(FacesMessage.SEVERITY_WARN,"subject.edit.nosubj");
       setInitError(true);
     }
  }

  public Subject getSubj() {
    return subj;
  }

  public void setSubj(Subject subj) {
    this.subj = subj;
  }

  public String done() throws AdminException {
    if(check(subj)) {
      if(oldName==null) {
        List<Subject> subjs = settings.getSubjects();
        subjs.add(subj);
        settings.setSubjects(subjs);
      }

      List<Address> masks = new ArrayList<Address>();
      for(DynamicTableRow row : masksModel.getRows()) {
        Address mask = new Address((String)row.getValue("mask"));
        masks.add(mask);
      }
      subj.setMasks(masks);

      List<String> children = new ArrayList<String>();
      for(DynamicTableRow row : subjectsModel.getRows()) {
        String child = (String)row.getValue("child");
        children.add(child);
      }
      subj.setChildren(children);


      setSettings(settings);
      return "SUBJECTS";

    }
    return null;
  }

  private boolean check(Subject subj) {
    if(oldName==null) {
      for(Subject s : settings.getSubjects()) {
        if(s.getName().equals(subj.getName())) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"subject.edit.subj.exists");
        }
      }
    }
    for(DynamicTableRow row : masksModel.getRows()) {
        String mask = (String)row.getValue("mask");
        if(!Address.validate(mask)) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN,"subject.edit.invlaid.mask",mask);
          return false;
        }
    }
    return true;
  }

  public String cancel() {
      return "SNMP";
  }

  public List<SelectItem> getSmeIds() throws AdminException {
      Set<String> smeIds = WebContext.getInstance().getSmeManager().smes().keySet();
      List<SelectItem> ret = new ArrayList<SelectItem>();
      for(String id : smeIds) {
        ret.add(new SelectItem(id));
      }
      return ret;
  }

  public DynamicTableModel getMasksModel() {
    return masksModel;
  }
  public void setMasksModel(DynamicTableModel masksModel ) {
    this.masksModel=masksModel;
  }

  public void setSubjectsModel(DynamicTableModel subjectsModel) {
    this.subjectsModel = subjectsModel;
  }

  public DynamicTableModel getSubjectsModel() {
    return subjectsModel;
  }

  public List getAllowedChildren() {
    List<String> ret = new ArrayList<String>();
    for(Subject s : settings.getSubjects()) {
      //todo filter not valid candidates
      if(!s.getName().equals(subj.getName())) {
            ret.add(s.getName());
      }
    }
    return ret;
  }
}