package mobi.eyeline.welcomesms.backend.datasources;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.admin.AdminException;
import org.apache.log4j.Category;

import java.util.Iterator;

import mobi.eyeline.welcomesms.backend.WelcomeSMSContext;
import mobi.eyeline.welcomesms.backend.Network;

/**
 * author: alkhal
 */
public class NetworkDataSource extends AbstractDataSource {

  private static final Category logger = Category.getInstance(NetworkDataSource.class);

  public static final String ID = "id";
  public static final String NAME = "name";
  public static final String COUNTRY_ID = "countryId";
  public static final String MCC = "mcc";
  public static final String MNC = "mnc";


  private final WelcomeSMSContext context;


  public NetworkDataSource(WelcomeSMSContext context) {
    super(new String[]{ID, NAME, COUNTRY_ID, MCC, MNC});
    this.context = context;
  }

  public QueryResultSet query(Query query_to_run) {
    init(query_to_run);
    try {
      for (Iterator iter = context.getNetworksManager().getNetworks().iterator(); iter.hasNext();) {
        Network t = (Network)iter.next();
        NetworkDataItem item = new NetworkDataItem(t.getId(), t.getName(), t.getMcc(), t.getMnc(), t.getCountryId());
        add(item);
      }
    } catch (AdminException e) {
      logger.error("Could not get networks", e);
    }

    return getResults();
  }

}
