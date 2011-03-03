package ru.novosoft.smsc.jsp.util.tables.impl.network_profiles;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.network_profiles.NetworkProfile;
import ru.novosoft.smsc.admin.network_profiles.NetworkProfilesManager;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;

import java.util.Iterator;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfilesDataSource extends AbstractDataSourceImpl {

  private static final Category log = Category.getInstance(NetworkProfilesDataSource.class);

  private final NetworkProfilesManager nm;

  public NetworkProfilesDataSource(SMSCAppContext appContext) {
    super(new String[]{"name", "abonentStatusMethod", "ussdOpenDestRef"});
    this.nm = appContext.getNetworkProfilesManager();
  }

  public QueryResultSet query(NetworkProfileQuery query) throws AdminException {
    try {
      data.clear();
      for (Iterator iter = nm.getProfiles().iterator(); iter.hasNext();)
        data.add(new NetworkProfileDataItem((NetworkProfile)iter.next()));

      return super.query(query);

    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException(e.getMessage());
    }

  }
}