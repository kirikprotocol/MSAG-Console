package ru.novosoft.smsc.web.controllers.closed_groups;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.util.Address;

import javax.faces.application.FacesMessage;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * @author alkhal
 */
public class ClosedGroupEditController extends ClosedGroupsController{

  private String id;

  private String description;

  private String name;

  private String oldName;

  private DynamicTableModel dynamicModel = new DynamicTableModel();

  public ClosedGroupEditController() {
    super();
    id = getRequestParameter("closed_group");

    if(id != null && id.length() > 0) {
      try{
        reload(Integer.parseInt(id));
      }catch (AdminException e){
        addError(e);
      }
    }
  }

  private void reload(int id) throws AdminException{
    ClosedGroup oldGroup = manager.getGroup(id);
    name = oldName = oldGroup.getName();
    description = oldGroup.getDescription();
    dynamicModel = new DynamicTableModel();
    for(Address a : oldGroup.getMasks()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("mask", a.getSimpleAddress());
      dynamicModel.addRow(row);
    }
  }


  public String save() {
    try{
      ClosedGroup cg;
      String user = getUserName();
      if(id != null && id.length()>0) {
        if(!oldName.equals(name)) {
          manager.removeGroup(Integer.parseInt(id));
          cg = manager.addGroup(name, description);
          id = Long.toString(cg.getId());
        }else {
          cg = manager.getGroup(Integer.parseInt(id));
          cg.setDescription(description);
        }
      }else {
        cg = manager.addGroup(name, description);
      }

      Set<String> newM = new HashSet<String>();

      for(DynamicTableRow r : dynamicModel.getRows()) {
        newM.add((String)r.getValue("mask"));
      }

      Map<String, Address> oldM = new HashMap<String, Address>();
      for(Address a :cg.getMasks()) {
        oldM.put(a.getSimpleAddress(), a);
      }

      for(Map.Entry<String, Address> old : oldM.entrySet()){
        if(!newM.contains(old.getKey())) {
          cg.removeMask(old.getValue());
        }
      }
      boolean warns = false;
      for(String nw : newM) {
        if(!oldM.containsKey(nw)) {
          Address a = null;
          try{
            a = new Address(nw);
          }catch (IllegalArgumentException e) {
            warns = true;
            addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.closed_group.mask.illegal", nw);
          }
          if(a != null) {
            cg.addMask(a);
          }
        }
      }

      if(warns) {
        if(id != null) {
          try{
            reload(Integer.parseInt(id));
          }catch (Exception ex) {
            logger.error(ex,ex);
          }
        }
        return null;
      }

      return "CLOSED_GROUPS";
    }catch (AdminException e){
      addError(e);
      if(id != null) {
        try{
          reload(Integer.parseInt(id));
        }catch (Exception ex) {
          logger.error(ex,ex);
        }
      }
      return null;
    }

  }


  public DynamicTableModel getDynamicModel() {
    return dynamicModel;
  }

  public void setDynamicModel(DynamicTableModel dynamicModel) {
    this.dynamicModel = dynamicModel;
  }


  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String description) {
    this.description = description;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }
}
