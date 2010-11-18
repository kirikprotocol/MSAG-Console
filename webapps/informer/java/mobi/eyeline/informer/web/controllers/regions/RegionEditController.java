package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class RegionEditController extends RegionsController {


  private static final Logger logger = Logger.getLogger(RegionEditController.class);

  private Integer id;

  private Region region;

  private DynamicTableModel dynamicModel = new DynamicTableModel();

  private final List<SelectItem> timeZones = new LinkedList<SelectItem>();

  private static final List<TimeZone> tZones = new LinkedList<TimeZone>();

  static {
    tZones.add(TimeZone.getTimeZone("Europe/Kaliningrad"));
    tZones.add(TimeZone.getTimeZone("Europe/Moscow"));
    tZones.add(TimeZone.getTimeZone("Europe/Volgograd"));
    tZones.add(TimeZone.getTimeZone("Europe/Samara"));
    tZones.add(TimeZone.getTimeZone("Asia/Yekaterinburg"));
    tZones.add(TimeZone.getTimeZone("Asia/Omsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Novosibirsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Novokuznetsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Krasnoyarsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Irkutsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Yakutsk"));
    tZones.add(TimeZone.getTimeZone("Asia/Vladivostok"));
    tZones.add(TimeZone.getTimeZone("Asia/Sakhalin"));
    tZones.add(TimeZone.getTimeZone("Asia/Magadan"));
    tZones.add(TimeZone.getTimeZone("Asia/Kamchatka"));
    tZones.add(TimeZone.getTimeZone("Asia/Anadyr"));
  }

  private final Collection<Smsc> ss;

  public RegionEditController() {
    super();

    String id = getRequestParameter(REGION_ID_PARAMETER);


    ss = getConfig().getSmscs();

    if (id != null && id.length() > 0) {
      this.id = Integer.parseInt(id);
      reload();
    } else {
      region = new Region();
      if (!ss.isEmpty()) {
        try {
          region.setSmsc(ss.iterator().next().getName());
        } catch (AdminException e) {
          addError(e);
        }
      }
    }


    Locale l = getLocale();
    for (TimeZone t : tZones) {
      timeZones.add(new SelectItem(t, t.getDisplayName(l)));
    }
  }

  private void reload() {
    region = getConfig().getRegion(id);
    if (region == null) {
      logger.warn("REGION is not found with id=" + id);
      id = null;
      region = new Region();
      if (!ss.isEmpty()) {
        try {
          region.setSmsc(ss.iterator().next().getName());
        } catch (AdminException e) {
          addError(e);
        }
      }
    } else {
      dynamicModel = new DynamicTableModel();
      for (Address a : region.getMasks()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("mask", a.getSimpleAddress());
        dynamicModel.addRow(row);
      }
    }
  }


  public String save() {
    if (dynamicModel.getRowCount() == 0) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "region.masks.empty");
      return null;
    }
    Configuration config = getConfig();
    String user = getUserName();

    List<Address> newMasks = new LinkedList<Address>();
    for (DynamicTableRow row : dynamicModel.getRows()) {
      String mask = (String) row.getValue("mask");
      if (!Address.validate(mask)) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "validation.msisdn");
        return null;
      }
      newMasks.add(new Address(mask));
    }

    try {
      region.clearMasks();
      for (Address a : newMasks) {
        region.addMask(a);
      }
    } catch (AdminException e) {
      addError(e);
      return null;
    }

    try {
      if (region.getRegionId() != null) {
        config.updateRegion(region, user);
      } else {
        config.addRegion(region, user);
      }

      return "REGION";
    } catch (AdminException e) {
      addError(e);
      return null;
    }

  }

  public List<SelectItem> getTimeZones() {
    return timeZones;
  }

  public List<SelectItem> getSmscs() {
    List<SelectItem> smscs = new ArrayList<SelectItem>(ss.size());
    for (Smsc s : ss) {
      smscs.add(new SelectItem(s.getName(), s.getName()));
    }
    return smscs;
  }

  public DynamicTableModel getDynamicModel() {
    return dynamicModel;
  }

  public void setDynamicModel(DynamicTableModel dynamicModel) {
    this.dynamicModel = dynamicModel;
  }

  public Region getRegion() {
    return region;
  }

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }
}
