package ru.novosoft.smsc.admin.profiler;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.blacknick.BlackNickDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.blacknick.BlackNickQuery;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNickDataFile {
  private static final Category log = Category.getInstance(BlackNickDataFile.class);

  private String listStorePath;
  private static final String[] columnNames = {"nick"};
  private static final String PARAM_NAME_FILE_NAME = "profiler.blackListFile";

  public void init(Config config) throws AdminException {
    try {
      listStorePath = config.getString(PARAM_NAME_FILE_NAME);
    } catch (Exception e) {
      throw new AdminException("Failed to obtain " + PARAM_NAME_FILE_NAME + ". Details: " + e.getMessage());
    }

    File statPath = new File(listStorePath);
    if (statPath == null || !statPath.isAbsolute()) {
      File smscConfFile = WebAppFolders.getSmscConfFolder();
      String absolutePath = smscConfFile.getParent();
      statPath = new File(absolutePath, listStorePath);
      log.debug("Sms store path: by smsc conf '" + statPath.getAbsolutePath() + "'");
    } else {
      log.debug("Sms store path: is absolute '" + statPath.getAbsolutePath() + "'");
    }
    listStorePath = statPath.getAbsolutePath();
  }

  private final static int ESC_CR = 13;
    private final static int ESC_LF = 10;

  private String readLine(InputStream bis) throws IOException {
    int ch = -1;
    final StringBuffer sb = new StringBuffer(30);
    while (true) {
      if ((ch = bis.read()) == -1) break;
      if (ch == ESC_CR) continue;
      else if (ch == ESC_LF) break;
      else sb.append((char)ch);
    }

    return (sb.length() == 0) ? null : sb.toString().trim();
  }



  public QueryResultSet query(BlackNickQuery query) throws AdminException {
    final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, query.getSortOrder());
    InputStream is = null;

    String line;
    try {
      is = new FileInputStream(listStorePath);

      int totalSize = 0;
      while ((line = readLine(is))!= null) {
        if (query.getFilter() != null && query.getFilter().length() > 0 && !line.startsWith(query.getFilter()))
          continue;

        rs.add(new BlackNickDataItem(line));
        totalSize++;
      }

      rs.setTotalSize(totalSize);;
      rs.sortByColumnName(query.getSortOrder());

    } catch (IOException e) {
      log.error("Can't read black nicks file", e);
      return new EmptyResultSet();
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }

    return rs;
  }

  private boolean isNickExists(final BlackNick nick) {
    InputStream is = null;

    String line;
    try {
      is = new FileInputStream(listStorePath);

      while ((line = readLine(is))!= null) {
        if (line.trim().equals(nick.getNick()))
          return true;
      }

      return false;

    } catch (IOException e) {
      log.error(e,e);
      return false;
    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
    }
  }

  public void addBlackNick(final BlackNick blackNick) throws AdminException {
    if (isNickExists(blackNick))
      throw new AdminException("Black nick " + blackNick.getNick() + " already exists");

    FileWriter writer = null;

    try {
      writer = new FileWriter(listStorePath, true);
      writer.write(blackNick.getNick() + "\n");
    } catch (IOException e) {
      throw new AdminException("Can't add nick: " + e.getMessage());
    } finally {
      try {
        if (writer != null)
          writer.close();
      } catch (IOException e) {
      }
    }
  }

  public void removeBlackNicks(final Collection blackNicks) throws AdminException {
    final HashSet nicks = new HashSet();
    for (Iterator iter = blackNicks.iterator(); iter.hasNext();)
      nicks.add(((BlackNick)iter.next()).getNick());

    final File listFile = new File(listStorePath);
    final File listFileBackup = new File(listStorePath + ".bak");

    listFile.renameTo(listFileBackup);

    InputStream is = null;
    FileWriter writer = null;

    String line;
    try {
      is = new FileInputStream(listFileBackup);
      writer = new FileWriter(listFile);

      while ((line = readLine(is))!= null) {
        if (!nicks.contains(line.trim()))
          writer.write(line + '\n');
      }

      writer.close();

      listFileBackup.delete();

    } catch (IOException e) {
      listFileBackup.renameTo(listFile);
      throw new AdminException("Can't read black nicks file. Reason: " + e.getMessage());

    } finally {
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
      }
      try {
        if (writer != null)
          writer.close();
      } catch (IOException e) {
      }
    }
  }

  public void removeBlackNick(final BlackNick blackNick) throws AdminException {
    final ArrayList list = new ArrayList();
    list.add(blackNick);
    removeBlackNicks(list);
  }
}
