package ru.novosoft.smsc.web.controllers.profile;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;
import mobi.eyeline.util.jsf.components.data_table.model.ModelWithObjectIds;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profile.Profile;
import ru.novosoft.smsc.admin.profile.ProfilesSet;
import ru.novosoft.smsc.util.Address;

import javax.faces.application.FacesMessage;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class ProfileGroupsController extends ProfileController {

  private String mask;
  private boolean applyCalled;
  private List<String> groups2remove;

  public String getMask() {
    return mask;
  }

  public void setMask(String mask) {
    this.mask = mask;
  }

  public boolean isApplyCalled() {
    return applyCalled;
  }

  public void setApplyCalled(boolean applyCalled) {
    this.applyCalled = applyCalled;
  }

  public void setGroups2remove(List<String> groups2remove) {
    this.groups2remove = groups2remove;
  }

  public DataTableModel getGroups() {
    return new ModelWithObjectIds() {
      private final List<Profile> result = new ArrayList<Profile>();

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        if (!applyCalled)
          return result;
        
        Collection<Profile> res;
        if (sortOrder == null)
          res = new LinkedList<Profile>();
        else
          res = new TreeSet<Profile>(new Comparator<Profile>() {
            public int compare(Profile o1, Profile o2) {
              int val = o1.getAddress().getSimpleAddress().compareTo(o2.getAddress().getSimpleAddress());
              return sortOrder.isAsc() ? val : -val;
            }
          });

        boolean filter = mask != null && mask.length() > 0;

        ProfilesSet pset = null;
        try {
          pset = mngr.getProfiles();

          while(pset.next()) {
            Profile p = pset.get();
            String address = p.getAddress().getSimpleAddress();
            if (address.contains("?") && (!filter || address.startsWith(mask)))
              res.add(pset.get());
          }

        } catch (AdminException e) {
          addError(e);
        } finally {
          if (pset != null)
            try {
              pset.close();
            } catch (AdminException e) {
            }
        }

        result.clear();

        int i=0;
        for (Profile p : res) {
          if (i < startPos)
            continue;
          if (i > startPos + count)
            break;

          result.add(p);
        }

        return result;
      }

      public int getRowsCount() {
        return result.size();
      }

      public String getId(Object o) throws ModelException {
        return ((Profile)o).getAddress().getSimpleAddress();
      }
    };
  }

  public String removeGroup() {
    if (groups2remove != null) {
      try {
        for (String group2remove : groups2remove)
          mngr.deleteProfile(new Address(group2remove));
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public void applyFilter() {
    applyCalled = true;
  }

  public void clearFilter() {
    applyCalled = false;
    mask = null;
  }
  

}
