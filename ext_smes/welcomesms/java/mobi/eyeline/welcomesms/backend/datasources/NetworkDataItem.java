package mobi.eyeline.welcomesms.backend.datasources;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * author: alkhal
 */
public class NetworkDataItem extends AbstractDataItem {

  public NetworkDataItem(Integer id, String name, int mcc, int mnc, String countryId) {
    values.put(NetworkDataSource.ID, id);
    values.put(NetworkDataSource.NAME, name);
    values.put(NetworkDataSource.MCC, new Integer(mcc));
    values.put(NetworkDataSource.MNC, new Integer(mnc));
    values.put(NetworkDataSource.COUNTRY_ID, countryId);
  }

  public Integer getId() {
    return (Integer) values.get(NetworkDataSource.ID);
  }

  public String getName() {
    return (String) values.get(NetworkDataSource.NAME);
  }

  public int getMcc() {
    return ((Integer) values.get(NetworkDataSource.MCC)).intValue();
  }

  public int getMnc() {
    return ((Integer) values.get(NetworkDataSource.MNC)).intValue();
  }

  public String getCountryId() {
    return (String) values.get(NetworkDataSource.COUNTRY_ID);
  }

}
