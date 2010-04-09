package mobi.eyeline.welcomesms.backend.datasources;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

/**
 * author: alkhal
 */
public class NetworkFilter implements Filter {

  private String countryId;

  private String name;

  private Integer mcc;

  private Integer mnc;

  private Integer id;


  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    NetworkDataItem d = (NetworkDataItem)item;
    if(id != null && d.getId() != id) {
      return false;
    }
    if(mcc != null && mcc.intValue() != d.getMcc() ) {
      return false;
    }
    if(mnc != null && mnc.intValue() != d.getMnc() ) {
      return false;
    }
    if(countryId != null && countryId.length() > 0 && !countryId.equals(d.getCountryId())) {
      return false;
    }
    if(name != null && d.getName() != null && !d.getName().startsWith(name)) {
      return false;
    }
    return true;
  }

  public String getCountryId() {
    return countryId;
  }

  public void setCountryId(String countryId) {
    this.countryId = countryId;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Integer getMcc() {
    return mcc;
  }

  public void setMcc(Integer mcc) {
    this.mcc = mcc;
  }

  public Integer getMnc() {
    return mnc;
  }

  public void setMnc(Integer mnc) {
    this.mnc = mnc;
  }

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }


}
