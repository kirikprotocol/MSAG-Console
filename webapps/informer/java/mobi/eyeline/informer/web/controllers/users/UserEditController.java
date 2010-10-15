package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.Set;
import java.util.TreeSet;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.10.2010
 * Time: 15:26:51
 */
public class UserEditController extends InformerController {

  private String userId;
  private User userToEdit;
  private static final String USER_ID_PARAMETER = "userId";
  private boolean initError;
  private DynamicTableModel regionsModel = new DynamicTableModel();
  private static final String INFORMER_ADMIN_ROLE = "informer-admin";
  private static final String INFORMER_USER_ROLE = "informer-user";

  public UserEditController() {
    super();
    try {
      setUserId(getRequestParameter(USER_ID_PARAMETER));
    }
    catch (AdminException e){
      initError = true;
    }
  }

  public String getUserId() {
    return userId;
  }

  private void setUserId(String userId) throws AdminException {
    if(userId==null || userId.length()==0) {
      userId=null;
      userToEdit = new User();
    }
    else {
      userToEdit = getConfig().getUser(userId);
    }
    initRegionsModel();
    this.userId = userId;
  }

  void initRegionsModel() {
    regionsModel = new DynamicTableModel();
    for(String region : userToEdit.getRegions()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("region",region);
      regionsModel.addRow(row);
    }
  }


  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public String save() {

    try {
      Set<String> regions = new TreeSet<String>();
      for(DynamicTableRow row : regionsModel.getRows()) {
        String regionId= (String) row.getValue("region");
        //todo check region exists
        regions.add(regionId);
      }
      userToEdit.setRegions(regions);
    }
    catch (AdminException e) {
      addError(e);
      return null;
    }

    return "USERS";  //To change body of created methods use File | Settings | File Templates.
  }

  public User getUserToEdit() {
    return userToEdit;
  }

  public DynamicTableModel getRegionsModel() {
    return regionsModel;
  }

  public void setRegionsModel(DynamicTableModel regionsModel) {
    this.regionsModel = regionsModel;
  }

  public boolean isAdmin() {
    return userToEdit.hasRole(INFORMER_ADMIN_ROLE);
  }

  public void setAdmin(boolean inRole) {
    if(inRole && !userToEdit.hasRole(INFORMER_ADMIN_ROLE)) {
      userToEdit.getRoles().add(INFORMER_ADMIN_ROLE);
      return;
    }
    if(!inRole && userToEdit.hasRole(INFORMER_ADMIN_ROLE)) {
      userToEdit.getRoles().remove(INFORMER_ADMIN_ROLE);
    }
  }


}