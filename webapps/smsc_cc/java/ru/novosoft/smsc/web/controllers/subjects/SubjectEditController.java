package ru.novosoft.smsc.web.controllers.subjects;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;


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
  String childToAdd;
  private DynamicTableModel masksModel;

  private Map<String, Subject> subjMap;

  public SubjectEditController() {
    super(WebContext.getInstance().getRouteSubjectManager());
    try {
      setOldName(getRequestParameter("oldName"));
      super.init();
      initData();
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

  private Map<String, Subject> buildSubjectsMap() {
    Map<String,Subject> subjMap = new HashMap<String,Subject>();
    for(Subject s : settings.getSubjects()) {
      subjMap.put(s.getName(),s);
    }
    return subjMap;
  }


  protected void initData() throws AdminException {
    settings = getSettings();
    subjMap = buildSubjectsMap();
    masksModel=new DynamicTableModel();

    if(oldName!=null) {
      subj = subjMap.get(oldName);
      if(subj!=null) {
        if(subj.getName().equals(oldName)) {
          for(Address mask : subj.getMasks()) {
            DynamicTableRow row = new DynamicTableRow();
            row.setValue("mask", mask.getSimpleAddress());
            masksModel.addRow(row);
          }
          for(String child : subj.getChildren()) {
            DynamicTableRow row = new DynamicTableRow();
            row.setValue("child", child);
          }
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

  public String done() {
    try {
      if(check(subj)) {
        if(oldName==null) {
          subjMap.put(subj.getName(),subj);
        }
        if(oldName!=null && !oldName.equals(subj.getName())) {
          subjMap.remove(oldName);
          for(Subject s : subjMap.values()) {
            List<String> children = s.getChildren();
            if(children.contains(oldName)) {
              children.remove(oldName);
              children.add(subj.getName());
              s.setChildren(children);
            }
          }
          subjMap.put(subj.getName(),subj);
        }

        List<Address> masks = new ArrayList<Address>();
        for(DynamicTableRow row : masksModel.getRows()) {
          Address mask = new Address((String)row.getValue("mask"));
          masks.add(mask);
        }
        subj.setMasks(masks);


        settings.setSubjects(new ArrayList<Subject>(subjMap.values()));
        setSettings(settings);
        setOldName(subj.getName());
        initData();
        return "SUBJECTS";
      }
    }
    catch (AdminException e) {
      addError(e);
    }       
    return null;
  }

  private boolean check(Subject subj) {
    if(oldName==null || !oldName.equals(subj.getName())) {
      if(subjMap.get(subj.getName())!=null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN,"subject.edit.subj.exists");
        return false;
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
    List<String> smeIds = new ArrayList<String>(WebContext.getInstance().getSmeManager().smes().keySet());
    Collections.sort(smeIds);
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


  public List<SelectItem> getAllowedChildren() {

    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(String sId : subjMap.keySet()) {
      String nameToCheck = subj.getName();
      if(nameToCheck!=null &&  nameToCheck.length()==0) nameToCheck=null;
      if(nameToCheck!=null) {
        if(checkContainsDescendant(subjMap,sId,nameToCheck)) {
          continue;
        }
        if(checkContainsDescendant(subjMap,nameToCheck,sId)) {
          continue;
        }
      }
      ret.add(new SelectItem(sId));
    }
    return ret;
  }

  public String getChildToAdd() {
    return childToAdd;
  }

  public void setChildToAdd(String childToAdd) {
    this.childToAdd = childToAdd;
  }

  public String removeChild() {
    String childToRemove = getRequestParameter("childToRemove");
    try {

    if(childToRemove!=null && childToRemove.length()>0) {
        List<String> children = subj.getChildren();
        if(children.contains(childToRemove)) {
          children.remove(childToRemove);
          subj.setChildren(children);
          settings.setSubjects(new ArrayList<Subject>(subjMap.values()));
          setSettings(settings);
          setOldName(subj.getName());
          initData();
        }
    }
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String addChild() {
    try {
      if(childToAdd !=null && childToAdd.length()>0) {
        List<String> children = subj.getChildren();
        if(!children.contains(childToAdd)) {
          children.add(childToAdd);
          childToAdd =null;
          subj.setChildren(children);
          settings.setSubjects(new ArrayList<Subject>(subjMap.values()));
          setSettings(settings);
          setOldName(subj.getName());
          initData();
        }
      }
    }
    catch (AdminException e) {
      addError(e);
    }
    return null;
  }


  private boolean checkContainsDescendant(Map<String, Subject> subjMap, String one, String other) {
    if(one.equals(other)) return true;
    for(String sId : subjMap.get(one).getChildren()) {
      Subject child = subjMap.get(sId);
      if(child!=null) {
        if(checkContainsDescendant(subjMap,child.getName(),other)){
          return true;
        }
      }
    }
    return false;
  }


}