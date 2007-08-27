package ru.novosoft.smsc.jsp.util.tables.impl.regions;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.RegionsManager;
import ru.novosoft.smsc.admin.region.Region;
import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.Set;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.HashSet;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsDataSource extends AbstractDataSourceImpl {

  private static final Category log = Category.getInstance(RegionsDataSource.class);

  private final RegionsManager rm;
  public RegionsDataSource(SMSCAppContext appContext) {
    super(new String[]{"name", "index", "send_speed"});
    this.rm = appContext.getRegionsManager();
  }

  public QueryResultSet query(RegionsQuery query) throws AdminException {

    try {
      // rm.reset();
      data.clear();
      for (Iterator iter = rm.getRegions().iterator(); iter.hasNext();)
        data.add(new RegionsDataItem((Region)iter.next()));

      return super.query(query);

    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException(e.getMessage());
    }

  }
}
