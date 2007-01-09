package ru.novosoft.smsc.jsp.util.tables.impl.alias;


import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;

import java.io.*;
import java.util.Vector;
import java.util.TreeSet;
import java.util.Comparator;

/**
 * User: artem
 * Date: 25.12.2006
 */
public class AliasDataSource implements DataSource{

  private static final Category logger = Category.getInstance(AliasDataSource.class);

  private static final int listSizeLimit = 2000;

  private static final int msgSizeLength = 48; //1+(1+1+21)+(1+1+21)+1

  public static final String ALIAS_FIELD = "Alias";
  public static final String ADDRESS_FIELD = "Address";
  public static final String HIDE_FIELD = "Hide";

  private final String aliasesFile;

  public AliasDataSource(String aliasesFile) {
    this.aliasesFile = aliasesFile;
  }

  private QueryResultSet readFile(Query query) throws AdminException {

    FileInputStream fis = null;

    final int start = query.getStartPosition();
    final int end = start + query.getExpectedResultsQuantity();
    int pos = 0;

    QueryResultSetImpl rs = new QueryResultSetImpl();
    boolean finish = false;

    try {
      fis = new FileInputStream(aliasesFile);

      Message.readString(fis, 8); // file name
      Message.readUInt32(fis); // version (int)

      QueryResultSetImpl tempList = null;

      while (!finish) {
        tempList = new QueryResultSetImpl(new String[] {ALIAS_FIELD, ADDRESS_FIELD, HIDE_FIELD}, "");

        try {
          while (pos < end && tempList.size() < listSizeLimit) {

            AliasDataItem new_alias = readNextAliadDataItem(fis);
            if (new_alias != null && query.getFilter().isItemAllowed(new_alias) && pos >= start) {
              tempList.add(new_alias);
              pos++;
            }

          }

          finish = (pos >= end);

        } catch (EOFException e) {
          finish = true;
        }

        rs.addAll(tempList);

        if (query.getAggregator() != null)
          rs = (QueryResultSetImpl)query.getAggregator().aggregateList(rs);

        if (rs.size() > listSizeLimit)
          throw new AdminException("Memory overflow");
      }

    } catch (Exception e) {
      logger.error("Unexpected exception occured reading alias set file", e);
      throw new AdminException("Error in reading aliases file");

    } finally {
      try {
        if (fis != null)
          fis.close();
      } catch (IOException e) {
        logger.warn("can't close file");
      }
    }

    rs.setTotalSize(pos);
    if (query.getSortOrder() != null)
      return makeOrder(query.getSortOrder(), rs);

    return rs;
  }

  private QueryResultSet makeOrder(final Vector sortOrder, QueryResultSetImpl rs) {
    final Comparator comparator = new Comparator() {
      public int compare(Object o, Object o1) {
        final DataItem item1 = (DataItem)o;
        final DataItem item2 = (DataItem)o1;

        for (int i=0; i < sortOrder.size(); i++) {
          String sortItem = (String)sortOrder.get(i);
          if (!sortItem.equals(ALIAS_FIELD) && !sortItem.equals(ADDRESS_FIELD) && !sortItem.equals(HIDE_FIELD))
            continue;
          
          final boolean isDecs = sortItem.startsWith("-");
          if (isDecs)
            sortItem = sortItem.substring(1);

          int result = ((String)item1.getValue(sortItem)).compareTo((String)item2.getValue(sortItem));
          if (isDecs)
            result = -result;

          if (result != 0)
            return result;
        }

        return 0;
      }
    };

    final TreeSet values = new TreeSet(comparator);

    for (int i=0; i<rs.size(); i++)
      values.add(rs.get(i));

    rs.clearValues();
    rs.addAll(values);
    rs.setTotalSize(values.size());
    values.clear();

    return rs;
  }

  private AliasDataItem readNextAliadDataItem(InputStream is) throws IOException {
    byte isInUse = (byte) Message.readUInt8(is);

    if (isInUse != 0) {

      byte addrTon = (byte) Message.readUInt8(is);
      byte addrNpi = (byte) Message.readUInt8(is);
      String addrValue = Message.readString(is, 21);
      byte aliasTon = (byte) Message.readUInt8(is);
      byte aliasNpi = (byte) Message.readUInt8(is);
      String aliasValue = Message.readString(is, 21);
      byte isHide = (byte) Message.readUInt8(is);

      return new AliasDataItem(new Mask(aliasTon, aliasNpi, aliasValue).getMask(), new Mask(addrTon, addrNpi, addrValue).getMask(), (isHide != 0));



    } else {
      // skip unused record
      for (int i=0; i < msgSizeLength - 1; i++)
        is.read();
      return null;
    }
  }

  public QueryResultSet query(Query query_to_run) throws AdminException {
    return readFile(query_to_run);
  }
}


