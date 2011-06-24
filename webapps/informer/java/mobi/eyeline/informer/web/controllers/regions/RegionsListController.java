package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.components.data_table.Identificator;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.InformerTimezone;

import javax.faces.model.SelectItem;
import java.io.PrintWriter;
import java.util.*;

/**
 * Контроллер для отображения списка регионов
 *
 * @author Aleksandr Khalitov
 */
public class RegionsListController extends RegionsController {

  private List<Region> regions;

  private List<String> selected;

  private String regionPrefix;

  private String smscPrefix;

  private Address address;

  private final List<SelectItem> smscs = new LinkedList<SelectItem>();

  private int defaultMax;


  public RegionsListController() {
    for (Smsc s : getConfig().getSmscs()) {
      smscs.add(new SelectItem(s.getName(), s.getName()));
    }
    Collections.sort(smscs, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    defaultMax = getConfig().getDefaultMaxPerSecond();
  }

  public String submitDefault() {
    try {
      getConfig().setDefaultMaxPerSecond(defaultMax, getUserName());
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String editGroup() {
    if(selected == null || selected.isEmpty()) {
      return null;
    }
    getRequest().put(REGION_IDS_PARAM, selected);
    selected = null;
    return "REGION_GROUP_EDIT";
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

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }

  private List<Region> getRegions() {
    if (regions == null) {
      regions = new LinkedList<Region>();
      if (address != null) {
        Region r = getConfig().getRegion(address);
        if (r != null) {
          regions.add(r);
        }
      } else {
        for (Region r : getConfig().getRegions()) {
          if (regionPrefix != null && (regionPrefix = regionPrefix.trim()).length() > 0) {
            if (!r.getName().startsWith(regionPrefix)) {
              continue;
            }
          }
          if (smscPrefix != null && (smscPrefix = smscPrefix.trim()).length() > 0) {
            if (!r.getSmsc().startsWith(smscPrefix)) {
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
  public void setSelected(List selected) {
    if (selected != null) {
      this.selected = new ArrayList<String>((List<String>) selected);
    }
  }


  public String remove() {
    if (selected != null) {
      for (String r : selected) {
        try {
          getConfig().removeRegion(Integer.parseInt(r), getUserName());
        } catch (AdminException e) {
          addError(e);
        }
      }
    }
    return null;
  }

  public DataTableModel getRegionsModel() {

    class DataTableModelImpl implements DataTableModel, Identificator {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<Region> regions = getRegions();

        List<Region> result = new ArrayList<Region>(count);

        if (count <= 0) {
          return result;
        }

        if (sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("name")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getName().compareTo(o2.getName())) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        } else if (sortOrder.getColumnId().equals("smsc")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getSmsc().compareTo(o2.getSmsc())) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        } else if (sortOrder.getColumnId().equals("maxPerSecond")) {
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (new Integer(o1.getMaxSmsPerSecond()).compareTo(o2.getMaxSmsPerSecond())) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        } else if (sortOrder.getColumnId().equals("timezone")) {
          final Locale locale = getLocale();
          Collections.sort(regions, new Comparator<Region>() {
            public int compare(Region o1, Region o2) {
              return (o1.getTimeZone().getDisplayName(locale).compareTo(o2.getTimeZone().getDisplayName(locale))) * (sortOrder.isAsc() ? 1 : -1);
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

      @Override
      public String getId(Object value) {
        return ((Region)value).getRegionId().toString();
      }

      public int getRowsCount() {
        return getRegions().size();
      }
    };
    return new DataTableModelImpl();
  }

  protected void _download(PrintWriter writer) {
    List<Region> regions = getRegions();

    for (Region r : regions) {
      InformerTimezone tz = WebContext.getInstance().getWebTimezones().getTimezoneByID(r.getTimeZone().getID());
      String tzName = tz.getAlias(getLocale());
      writer.println(StringEncoderDecoder.toCSVString(';',r.getName(), r.getSmsc(), r.getMaxSmsPerSecond(), tzName));
      for (Address addr : r.getMasks())
        writer.println(addr.getSimpleAddress());
    }

    writer.flush();
  }

}
