/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:52:19
 */
package ru.novosoft.smsc.admin.alias;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.jsp.util.tables.*;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;
import ru.novosoft.smsc.util.config.Config;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;


public class AliasSet {
//  private Set aliases = new HashSet();
  private AliasDataSource dataSource;
  private static final Category logger = Category.getInstance(AliasSet.class);

  private String aliasmanStorePath;
//  private File aliasesFile;
//  private long lastModified = 0;
  private static final String PARAM_NAME_FILE_NAME = "aliasman.storeFile";

  private Smsc smsc = null;

  public AliasSet() {
  }

  /**
   *
   * @param smsc
   * @throws AdminException
   */
  public void init(Config config, Smsc smsc) throws AdminException {
    try {
      aliasmanStorePath = config.getString(PARAM_NAME_FILE_NAME);
//      aliasmanStorePath = "/home/artem/Work/mnt/shulga/build/smsc/distr/services/SMSC/store";
      if (aliasmanStorePath == null || aliasmanStorePath.length() <= 0)
        throw new AdminException("store path is empty");
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException("Failed to obtain " + PARAM_NAME_FILE_NAME + " Details: " + e.getMessage());
    }
    dataSource = new AliasDataSource(aliasmanStorePath);
//    aliasesFile = new File(aliasmanStorePath);
    this.smsc = smsc;
  }

//  private void loadFile() {
//    dataSource.clear();
//    aliases.clear();
//    FileInputStream fis = null;
//    try {
//      fis = new FileInputStream(aliasesFile);
//      lastModified = aliasesFile.lastModified();
//      /*String FileName = */Message.readString(fis, 8);
//      /*int version = (int) */Message.readUInt32(fis);
//      int msgSizeLength = 48; //1+(1+1+21)+(1+1+21)+1
//      byte buf[] = new byte[48];
//
//      while (true) {
//        int read = 0;
//        while (read < msgSizeLength) {
//          int result = fis.read(buf, read, msgSizeLength - read);
//          if (result < 0) throw new EOFException("Failed to read " + msgSizeLength + " bytes, read failed at " + read);
//          read += result;
//        }
//        InputStream bis = new ByteArrayInputStream(buf, 0, msgSizeLength);
//
//        byte isInUse = (byte) Message.readUInt8(bis);
//        byte addrTon = (byte) Message.readUInt8(bis);
//        byte addrNpi = (byte) Message.readUInt8(bis);
//        String addrValue = Message.readString(bis, 21);
//        byte aliasTon = (byte) Message.readUInt8(bis);
//        byte aliasNpi = (byte) Message.readUInt8(bis);
//        String aliasValue = Message.readString(bis, 21);
//        byte isHide = (byte) Message.readUInt8(bis);
//
//        if (isInUse != 0) {
//          Alias new_alias = new Alias(new Mask(addrTon, addrNpi, addrValue), new Mask(aliasTon, aliasNpi, aliasValue), (isHide != 0));
//          dataSource.add(new_alias);
//          aliases.add(new_alias);
//        }
//
//        bis.close();
//      }
//    } catch (EOFException e) {
//    } catch (Exception e) {
//      logger.error("Unexpected exception occured reading alias set file", e);
//    } finally {
//      if (fis != null)
//        try {
//          fis.close();
//        } catch (IOException e) {
//          logger.warn("can't close file");
//        }
//    }
//  }

/*    public PrintWriter store(final PrintWriter out) {
        for (Iterator i = iterator(); i.hasNext();) {
            final Alias a = (Alias) i.next();
            out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask()) + "\" alias=\""
                    + StringEncoderDecoder.encode(a.getAlias().getMask())
                    + "\" hide=\""
                    + (a.isHide() ? "true" : "false")
                    + "\"/>");
        }
        return out;
    }*/

  public boolean add(final Alias new_alias) throws AdminException {
      smsc.addAlias(new_alias.getAddress().getMask(), new_alias.getAlias().getMask(), new_alias.isHide());
//      dataSource.add(new_alias);
//      return aliases.add(new_alias);
      return true;
  }

//  public Iterator iterator() {
//    return aliases.iterator();
//  }

  public boolean remove(final Alias a) {
    try {
      smsc.delAlias(a.getAlias().getMask());
//      dataSource.remove(a);
//      return aliases.remove(a);
      return true;
    }
    catch (AdminException e) {
      logger.error("Couldn't remove alias \"" + a.getAlias().getMask() + '"', e);
      return false;
    }
  }

  public boolean remove(final String alias) {
    try {
      final Alias a = new Alias(new Mask(alias), new Mask(alias), false);
      return remove(a);
    } catch (AdminException e) {
      logger.error("Couldn't remove alias \"" + alias + '"', e);
      return false;
    }
  }

  public QueryResultSet query(final AliasQuery query) throws AdminException {
//    if( aliasesFile.lastModified() > lastModified || lastModified == 0) {
//      loadFile();
//    }
//    dataSource.clear();
//    for (Iterator i = aliases.iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      dataSource.add(alias);
//    }
    return dataSource.query(query);
  }

//  public boolean contains(final Alias a) {
//    return aliases.contains(a);
//  }

  private Alias createAliasByDataItem(DataItem dataItem) throws AdminException {
    return new Alias(new Mask((String)dataItem.getValue(AliasDataSource.ADDRESS_FIELD)),
                     new Mask((String)dataItem.getValue(AliasDataSource.ALIAS_FIELD)),
                     ((Boolean)dataItem.getValue(AliasDataSource.HIDE_FIELD)).booleanValue());
  }

  public List findAliases(Filter filter, int resultsSize) throws AdminException {
    final List result = new ArrayList();
    final QueryResultSet rs = dataSource.query(new AliasQuery(resultsSize, filter, AliasDataSource.ALIAS_FIELD, 0));
    for (int i=0; i< rs.size(); i++)
      result.add(createAliasByDataItem(rs.get(i)));

    return result;
  }

  private Alias findAlias(Filter filter, Aggregator aggregator) throws AdminException {
    final AliasQuery query = new AliasQuery(1, filter, AliasDataSource.ALIAS_FIELD, 0);
    query.setAggregator(aggregator);
    final QueryResultSet rs = dataSource.query(query);
    return (rs.size() > 0) ? createAliasByDataItem(rs.get(0)) : null;
  }



  public Alias get(final String aliasString) throws AdminException {
//    for (Iterator i = aliases.iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      if (alias.getAlias().getMask().equals(aliasString))
//        return alias;
//    }
//    return null;
    final Filter filter = new Filter() {
      public boolean isEmpty() { return false; }

      public boolean isItemAllowed(DataItem item) {
        try {
          return createAliasByDataItem(item).getAlias().getMask().equals(aliasString);
        } catch (AdminException e) {
          return false;
        }
      }
    };

    return findAlias(filter, null);
  }

  public boolean isContainsAlias(final Mask aliasMask) throws AdminException {
//    for (Iterator i = aliases.iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      if (alias.getAlias().addressConfirm(aliasMask))
//        return true;
//    }
//    return false;

    final Filter filter = new Filter() {

      public boolean isEmpty() { return false; }

      public boolean isItemAllowed(DataItem item) {
        try {
          return createAliasByDataItem(item).getAlias().addressConfirm(aliasMask);
        } catch (AdminException e) {
          return false;
        }
      }
    };

    return findAlias(filter, null) != null;
  }


  public Alias getAliasByAddress(final Mask address) throws AdminException {
//    Alias result = null;
//    for (Iterator i = aliases.iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      if (alias.isHide()) {
//        if (alias.getAddress().addressConfirm(address)) {
//          if (null == result)
//            result = alias;
//          else if (alias.getAddress().getQuestionsCount() < result.getAddress().getQuestionsCount())
//            result = alias;
//        }
//      }
//    }
//    return result;
    final Aggregator aggregator = new AggregatorMax(new Comparator() {
      public int compare(Object o, Object o1) {
        try {
          final Alias alias1 = createAliasByDataItem((DataItem) o);
          final Alias alias2 = createAliasByDataItem((DataItem) o1);

          return (alias1.getAddress().getQuestionsCount() < alias2.getAddress().getQuestionsCount()) ? 1 : -1;

        } catch (AdminException e) {
          e.printStackTrace();
          return 0;
        }
      }
    });

    final Filter filter = new Filter() {
      public boolean isEmpty() { return false; }

      public boolean isItemAllowed(DataItem item) {
        try {
          final Alias alias = createAliasByDataItem(item);
          return alias.isHide() && alias.getAddress().addressConfirm(address);
        } catch (AdminException e) {
          return false;
        }
      }
    };

    return findAlias(filter, aggregator);
  }

  public Alias getAddressByAlias(final Mask aliasToSearch) throws AdminException {
//    Alias result = null;
//    for (Iterator i = aliases.iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      if (alias.getAlias().addressConfirm(aliasToSearch)) {
//        if (null == result)
//          result = alias;
//        else if (alias.getAlias().getQuestionsCount() < result.getAlias().getQuestionsCount())
//          result = alias;
//      }
//    }
//    return result;

    final Aggregator aggregator = new AggregatorMax(new Comparator() {
      public int compare(Object o, Object o1) {
        try {
          final Alias alias1 = createAliasByDataItem((DataItem) o);
          final Alias alias2 = createAliasByDataItem((DataItem) o1);

          return (alias1.getAddress().getQuestionsCount() < alias2.getAddress().getQuestionsCount()) ? 1 : -1;

        } catch (AdminException e) {
          e.printStackTrace();
          return 0;
        }
      }
    });

    final Filter filter = new Filter() {
      public boolean isEmpty() { return false; }

      public boolean isItemAllowed(DataItem item) {
        try {
          final Alias alias = createAliasByDataItem(item);
          return  alias.getAlias().addressConfirm(aliasToSearch);
        } catch (AdminException e) {
          return false;
        }
      }
    };


    return findAlias(filter, aggregator);
  }

  public Mask dealias(final Mask alias) throws AdminException {
    final Alias addressCandidat = getAddressByAlias(alias);
    if (null != addressCandidat) {
      final Mask result = addressCandidat.getAddress();
      final int questionsCount = result.getQuestionsCount();
      if (0 < questionsCount) {
        final String mask = result.getMask();
        final String sourceMask = alias.getMask();
        return new Mask(mask.substring(0, mask.length() - questionsCount) + sourceMask.substring(sourceMask.length() - questionsCount));
      } else
        return result;
    } else
      return null;
  }

  public boolean isAddressExistsAndHide(final Mask address, final Alias except) throws AdminException {
//    for (Iterator i = iterator(); i.hasNext();) {
//      final Alias alias = (Alias) i.next();
//      if (alias.isHide() && alias.getAddress().equals(address) && null != except && !except.equals(alias))
//        return true;
//    }
//    return false;

    final Filter filter = new Filter() {
      public boolean isEmpty() { return false; }

      public boolean isItemAllowed(DataItem item) {
        try {
          final Alias alias = createAliasByDataItem(item);
          return alias.isHide() && alias.getAddress().equals(address) && null != except && !except.equals(alias);
        } catch (AdminException e) {
          return false;
        }
      }
    };

    return findAlias(filter, null) != null;
  }
}
