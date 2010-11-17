package mobi.eyeline.informer.web.controllers.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.users.User;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 08.11.2010
 * Time: 14:49:06
 */
public class RestrictionEditController extends RestrictionController{
  private Integer restrictionId;
  private Restriction restriction;
  private static final String ID_PARAMETER = "restrictionId";
  private boolean initError;



  public RestrictionEditController() {
    try {
      String s = getRequestParameter(ID_PARAMETER);
      setRestrictionId(s==null || s.length()==0 ? null :Integer.valueOf(s));
    }
    catch (AdminException e){
      initError = true;
    }
  }


  public Integer getRestrictionId() {
    return restrictionId;
  }

  private void setRestrictionId(Integer restrictionId) throws AdminException {
    if(restrictionId==null ) {
      restriction = new Restriction();
    }
    else {
      restriction = getConfig().getRestriction(restrictionId);
    }
    this.restrictionId = restrictionId;
  }


  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String save() {
    try {
      if(restrictionId==null) {
        getConfig().addRestriction(restriction,getUserName());
      }
      else {
        getConfig().updateRestriction(restriction,getUserName());
      }
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }

    return "RESTRICTIONS";  //To change body of created methods use File | Settings | File Templates.
  }

  public Restriction getRestriction() {
    return restriction;
  }


  public void setSelectedUsers(List<String> selectedUsers) {
    restriction.setUserIds(selectedUsers);
  }

  public List<String> getSelectedUsers() {
    if(restriction.getUserIds()==null) restriction.setUserIds(new ArrayList<String>());
    return restriction.getUserIds();
  }

  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();    
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }
}
