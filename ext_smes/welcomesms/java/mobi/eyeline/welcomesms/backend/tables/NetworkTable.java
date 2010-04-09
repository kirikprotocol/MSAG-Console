package mobi.eyeline.welcomesms.backend.tables;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import java.util.*;

import mobi.eyeline.welcomesms.backend.datasources.NetworkDataSource;
import mobi.eyeline.welcomesms.backend.datasources.NetworkQuery;
import mobi.eyeline.welcomesms.backend.datasources.NetworkDataItem;
import mobi.eyeline.welcomesms.backend.WelcomeSMSContext;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 */
public class NetworkTable extends PagedStaticTableHelper {

  private final TextColumn checkColumn = new TextColumn("checkColumn", "", false);
  private final TextColumn nameColumn = new TextColumn(NetworkDataSource.NAME, "welcomesms.network.name", true, 35);
  private final TextColumn countryColumn = new TextColumn(NetworkDataSource.COUNTRY_ID, "welcomesms.network.country", true, 35);
  private final TextColumn mccColumn = new TextColumn(NetworkDataSource.MCC, "welcomesms.network.mcc", true, 10);
  private final TextColumn mncColumn = new TextColumn(NetworkDataSource.MNC, "welcomesms.network.mnc", true, 10);

  private DataSource ds;

  private int totalSize = 0;

  private String sortOrder = "";

  private Filter filter;

  private WelcomeSMSContext context;


  public NetworkTable(String uid) {
    super(uid, false);

    addColumn(checkColumn);
    addColumn(nameColumn);
    addColumn(countryColumn);
    addColumn(mccColumn);
    addColumn(mncColumn);
  }

  public void setContext(WelcomeSMSContext context) {
    this.context = context;
  }

  private void buildSortOrder() {
    SortOrderElement[] sortOrderElements = getSortOrder();
    if ((sortOrderElements != null) && (sortOrderElements.length > 0)) {
      SortOrderElement element = sortOrderElements[0];
      if (element != null) {
        sortOrder = element.getColumnId();
        if (sortOrder != null) {
          if (element.getOrderType() != OrderType.ASC) {
            sortOrder = "-" + sortOrder;
          }
          return;
        }
      }
    }
    sortOrder = NetworkDataSource.NAME;
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    try {
      buildSortOrder();
      final QueryResultSet quizesList = ds.query(new NetworkQuery(size, filter, sortOrder, start));

      for (int i = 0; i < quizesList.size(); i++) {
        final NetworkDataItem item = (NetworkDataItem) quizesList.get(i);

        final Row row = createNewRow();

        final Integer id = item.getId();
        final String name = item.getName();
        final String countryId = item.getCountryId();
        final int mcc = item.getMcc();
        final int mnc = item.getMnc();

        row.addCell(checkColumn, new CheckBoxCell("chb" + id, false));
        row.addCell(nameColumn, new StringCell(id.toString(), name, true));
        row.addCell(countryColumn, new StringCell(id.toString(), getCountry(countryId), false));
        row.addCell(mccColumn, new StringCell(id.toString(), Integer.toString(mcc), false));
        row.addCell(mncColumn, new StringCell(id.toString(), Integer.toString(mnc), false));
      }
      totalSize = quizesList.getTotalSize();

    } catch (Exception e) {
      throw new TableHelperException(e);
    }
  }

  private String getCountry(String id) {
    if(context != null) {
      return context.getLocationService().getCountryById(id);
    }
    return null;
  }

  public List getSelected(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext();) {
      final String paramName = (String) iter.next();
      if (paramName.startsWith("chb"))
        result.add(Integer.valueOf(paramName.substring(3)));
    }
    return result;
  }

  protected int calculateTotalSize() throws TableHelperException {
    return totalSize;
  }

  public int getTotalSize() {
    return totalSize;
  }

  public void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }


  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public Filter getFilter() {
    return filter;
  }

  public void setFilter(Filter filter) {
    this.filter = filter;
  }
}
