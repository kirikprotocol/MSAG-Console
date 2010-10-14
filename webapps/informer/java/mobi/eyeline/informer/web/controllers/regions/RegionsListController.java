package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * Контроллер для отображения списка регионов
 * @author Aleksandr Khalitov
 */
public class RegionsListController extends RegionsController{

  private List<Region> regions;

  private List<String> toRemove;

  private String regionPrefix;

  private String smscPrefix;

  private String address;

  private List<SelectItem> smscs = new LinkedList<SelectItem>();
  
  private int defaultMax;


  public RegionsListController() {
    for(Smsc s : getConfig().getSmscs()) {
      smscs.add(new SelectItem(s.getName(), s.getName()));
    }
    defaultMax = getConfig().getDefaultMaxPerSecond();
  }

  public String submitDefault() {
    try{
      getConfig().setDefaultMaxPerSecond(defaultMax, getUserName());
    }catch (AdminException e){
      addError(e);
    }
    return null;
  }

  public List<SelectItem> getSmscs() {
    return smscs;
  }

  public int getDefaultMax() {
    return defaultMax;
  }

  public void setDefaultMax(int defaultMax) {
    this.defaultMax = defaultMax;
  }

  public void clearFilter() {
    regionPrefix = null;
    smscPrefix = null;
    address = null;
  }

  public String getRegionPrefix() {
    return regionPrefix;
  }

  public void setRegionPrefix(String regionPrefix) {
    this.regionPrefix = regionPrefix;
  }

  public String getSmscPrefix() {
    return smscPrefix;
  }

  public void setSmscPrefix(String smsc) {
    this.smscPrefix = smsc;
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  private List<Region> getRegions() {
    if(regions == null) {
      regions = new LinkedList<Region>();
      if(address != null && (address = address.trim()).length() > 0) {
        if(!Address.validate(address)) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "validation.msisdn"); 
        }else {
          Region r = getConfig().getRegion(new Address(address));
          if(r != null) {
            regions.add(r);
          }
        }
      }else {
        for(Region r : getConfig().getRegions()) {
          if(regionPrefix != null && (regionPrefix = regionPrefix.trim()).length()>0) {
            if(!r.getName().startsWith(regionPrefix)) {
              continue;
            }
          }
          if(smscPrefix != null && (smscPrefix = smscPrefix.trim()).length()>0) {
            if(!r.getSmsc().startsWith(smscPrefix)) {
              continue;
            }
          }
          regions.add(r);
        }
      }
    }
    return regions;
  }

  @SuppressWarnings({"unchecked"})
  public void setToRemove(List toRemove) {
    if(toRemove != null) {
      this.toRemove = new ArrayList<String>((List<String>)toRemove);
    }
  }


  public String remove() {
    if(toRemove != null) {
      for(String r : toRemove) {
        try{
          getConfig().removeRegion(r, getUserName());
        }catch (AdminException e){
          addError(e);
        }
      }
    }
    return null;
  }

  public DataTableModel getRegionsModel() {

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<Region> regions = getRegions();

        List<Region> result = new ArrayList<Region>(count);

        if (count <= 0) {
          return result;
        }

        if(sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("name")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getName().compareTo(o2.getName())) *( sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("smsc")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getSmsc().compareTo(o2.getSmsc())) *(sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("maxPerSecond")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (new Integer(o1.getMaxSmsPerSecond()).compareTo(o2.getMaxSmsPerSecond())) *(sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("timezone")) {
          final Locale locale = getLocale();
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getTimeZone().getDisplayName(locale).compareTo(o2.getTimeZone().getDisplayName(locale))) *(sortOrder.isAsc() ? 1 : -1);
            }
          });
        }


        for (Iterator<Region> i = regions.iterator(); i.hasNext() && count > 0;) {
          Region r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }

        return result;
      }

      public int getRowsCount() {
        return getRegions().size();
      }
    };
  }

}
