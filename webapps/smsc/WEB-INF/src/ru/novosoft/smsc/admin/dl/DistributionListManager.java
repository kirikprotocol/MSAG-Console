/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 17, 2003
 * Time: 1:43:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.exceptions.*;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;

import javax.sql.DataSource;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.*;

public class DistributionListManager implements DistributionListAdmin
{
  private final static String GET_PRINCIPALS_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS";
  private final static String CHECK_PRINCIPAL_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_PRINCIPALS WHERE ADDRESS=?";
  private final static String ADD_PRINCIPAL_SQL = "INSERT INTO DL_PRINCIPALS (ADDRESS, MAX_LST, MAX_EL) VALUES (?, ?, ?)";
  private final static String CHECK_PRINCIPAL_SUB_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE ADDRESS=?";
  private final static String DELETE_PRINCIPAL_SQL = "DELETE FROM DL_PRINCIPALS WHERE ADDRESS=?";
  private final static String GET_PRINCIPAL_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS=?";
  private final static String ALT_PRINCIPAL_LIST_SQL = "UPDATE DL_PRINCIPALS SET MAX_LST=? WHERE ADDRESS=?";
  private final static String ALT_PRINCIPAL_ELEM_SQL = "UPDATE DL_PRINCIPALS SET MAX_EL=? WHERE ADDRESS=?";
  private final static String CHECK_MEMBER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=? AND ADDRESS=?";
  private final static String ADD_MEMBER_SQL = "INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (?, ?)";
  private final static String DELETE_MEMBER_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=? AND ADDRESS= ?";
  private final static String GET_MEMBERS_SQL = "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=?";
  private final static String CHECK_SUBMITTER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
  private final static String ADD_SUBMITTER_SQL = "INSERT INTO DL_SUBMITTERS (LIST, ADDRESS) VALUES (?, ?)";
  private final static String DELETE_SUBMITTER_SQL = "DELETE FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
  private final static String GET_LIST_PRINCIPALS_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS IN (SELECT ADDRESS FROM DL_SUBMITTERS WHERE LIST=?)";
  private final static String CHECK_DL_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=?";
  private final static String ADD_DL_SQL = "INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (?, ?, ?)";
  private final static String DELETE_MEMBERS_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=?";
  private final static String DELETE_DL_SUB_SQL = "DELETE FROM DL_SUBMITTERS WHERE LIST=?";
  private final static String DELETE_DL_SQL = "DELETE FROM DL_SET WHERE LIST=?";
  private final static String GET_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET WHERE LIST=?";
  private final static String LIST_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET";
  private final static String ALT_DL_SQL = "UPDATE DL_SET SET MAX_EL=? WHERE LIST=?";
  private final static String GET_DL_SQL_FOR_UPDATE = "SELECT MAX_EL, OWNER FROM DL_SET WHERE LIST=?"; // FOR UPDATE
  private final static String GET_DLMEMBERS_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=?";
  private final static String GET_DLS_FOR_OWNER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE OWNER=?";
  private final static String GET_MAX_MEMBERS_COUNT_FOR_OWNER_SQL = "SELECT NVL(MAX(COUNT(LIST)), 0) FROM DL_MEMBERS WHERE LIST IN (SELECT LIST FROM DL_SET WHERE OWNER=?) GROUP BY LIST";


  private DataSource ds;
  private Category logger = Category.getInstance(this.getClass());

  public DistributionListManager(DataSource ds)
  {
    this.ds = ds;
  }

  public List principals() throws AdminException
  {
    List list = new ArrayList();
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_PRINCIPALS_SQL);
      rs = stmt.executeQuery();
      while (rs.next()) {
        list.add(new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3)));
      }
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return list;
  }

  public void addPrincipal(Principal prc) throws AdminException, PrincipalAlreadyExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
      stmt.setString(1, prc.getAddress());
      rs = stmt.executeQuery();
      if (rs.next() && rs.getInt(1) != 0)
        throw new PrincipalAlreadyExistsException(prc);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(ADD_PRINCIPAL_SQL);
      stmt.setString(1, prc.getAddress());
      stmt.setInt(2, prc.getMaxLists());
      stmt.setInt(3, prc.getMaxElements());
      stmt.executeUpdate();
      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public void deletePrincipal(String address) throws AdminException, PrincipalNotExistsException, PrincipalInUseException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
      stmt.setString(1, address);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new PrincipalNotExistsException(address);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_PRINCIPAL_SUB_SQL);
      stmt.setString(1, address);
      rs = stmt.executeQuery();
      if (rs.next() && rs.getInt(1) != 0)
        throw new PrincipalInUseException(address);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(DELETE_PRINCIPAL_SQL);
      stmt.setString(1, address);
      stmt.executeUpdate();
      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public Principal getPrincipal(String address) throws AdminException, PrincipalNotExistsException
  {
    Principal prc = null;
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_PRINCIPAL_SQL);
      stmt.setString(1, address);
      rs = stmt.executeQuery();
      if (!rs.next())
        throw new PrincipalNotExistsException(address);
      prc = new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3));
      rs.close();
      rs = null;
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return prc;
  }

  public void alterPrincipal(Principal prc, boolean altLists, boolean altElements)
          throws AdminException, PrincipalNotExistsException, ListsCountExceededException,
          MembersCountExceededForOwnerException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
      stmt.setString(1, prc.getAddress());
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new PrincipalNotExistsException(prc.getAddress());
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(GET_DLS_FOR_OWNER_SQL);
      stmt.setString(1, prc.getAddress());
      rs = stmt.executeQuery();
      int listsCount = rs.next() ? rs.getInt(1) : 0;
      rs.close();
      rs = null;
      stmt.close();

      if (listsCount > prc.getMaxLists())
        throw new ListsCountExceededException(prc.getAddress());

      stmt = connection.prepareStatement(GET_MAX_MEMBERS_COUNT_FOR_OWNER_SQL);
      stmt.setString(1, prc.getAddress());
      rs = stmt.executeQuery();
      int maxMembersCount = rs.next() ? rs.getInt(1) : 0;
      rs.close();
      rs = null;
      stmt.close();

      if (maxMembersCount > prc.getMaxElements())
        throw new MembersCountExceededForOwnerException(prc.getAddress(), maxMembersCount, prc.getMaxElements());

      if (altLists) {
        stmt = connection.prepareStatement(ALT_PRINCIPAL_LIST_SQL);
        stmt.setInt(1, prc.getMaxLists());
        stmt.setString(2, prc.getAddress());
        stmt.executeUpdate();
        stmt.close();
      }
      if (altElements) {
        stmt = connection.prepareStatement(ALT_PRINCIPAL_ELEM_SQL);
        stmt.setInt(1, prc.getMaxElements());
        stmt.setString(2, prc.getAddress());
        stmt.executeUpdate();
        stmt.close();
      }
      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public void addMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberAlreadyExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_DL_SQL_FOR_UPDATE);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next())
        throw new ListNotExistsException(dlname);
      int maxMembers = rs.getInt(1);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, address);
      rs = stmt.executeQuery();
      if (rs.next() && rs.getInt(1) != 0)
        throw new MemberAlreadyExistsException(address, dlname);
      rs.close();
      rs = null;
      stmt.close();


      stmt = connection.prepareStatement(GET_DLMEMBERS_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      int membersCount = rs.next() ? rs.getInt(1) : 0;
      rs.close();
      rs = null;
      stmt.close();
      if (membersCount >= maxMembers)
        throw new MembersCountExceededException(dlname, membersCount, maxMembers);

      stmt = connection.prepareStatement(ADD_MEMBER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, address);
      stmt.executeUpdate();
      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public void deleteMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberNotExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, address);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new MemberNotExistsException(address, dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(DELETE_MEMBER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, address);
      stmt.executeUpdate();
      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public List members(String dlname) throws AdminException, ListNotExistsException
  {
    List list = new ArrayList();
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(GET_MEMBERS_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      while (rs.next()) list.add(rs.getString(1));
      rs.close();
      rs = null;
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return list;
  }

  public void grantPosting(String dlname, String submitter) throws AdminException, ListNotExistsException, PrincipalNotExistsException, SubmitterAlreadyExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
      stmt.setString(1, submitter);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new PrincipalNotExistsException(submitter);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, submitter);
      rs = stmt.executeQuery();
      if (rs.next() && rs.getInt(1) != 0)
        throw new SubmitterAlreadyExistsException(submitter, dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(ADD_SUBMITTER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, submitter);
      stmt.executeUpdate();

      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public void revokePosting(String dlname, String submitter) throws AdminException, ListNotExistsException, SubmitterNotExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, submitter);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new SubmitterNotExistsException(submitter, dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(DELETE_SUBMITTER_SQL);
      stmt.setString(1, dlname);
      stmt.setString(2, submitter);
      stmt.executeUpdate();

      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public List submitters(String dlname) throws AdminException, ListNotExistsException
  {
    List list = new ArrayList();
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(GET_LIST_PRINCIPALS_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      while (rs.next()) {
        list.add(new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3)));
      }
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return list;
  }

  public void addDistributionList(DistributionList dl) throws AdminException, ListAlreadyExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dl.getName());
      rs = stmt.executeQuery();
      if (rs.next() && rs.getInt(1) != 0)
        throw new ListAlreadyExistsException(dl.getName());
      rs.close();
      rs = null;
      stmt.close();

      if (!dl.isSys()) {
        stmt = connection.prepareStatement(GET_PRINCIPAL_SQL); //ADDRESS, MAX_LST, MAX_EL
        stmt.setString(1, dl.getNormalizedOwner());
        rs = stmt.executeQuery();
        if (!rs.next())
          throw new OwnerNotExistsException(dl.getName(), dl.getOwner());
        int maxLst = rs.getInt(2);
        int maxMembers = rs.getInt(3);
        rs.close();
        rs = null;
        stmt.close();

        if (maxMembers < dl.getMaxElements())
          throw new DistListMaxMembersGreaterThanOwnerMaxMembers(dl.getName(), dl.getOwner(), dl.getMaxElements(), maxMembers);


        stmt = connection.prepareStatement(GET_DLS_FOR_OWNER_SQL); //ADDRESS, MAX_LST, MAX_EL
        stmt.setString(1, dl.getNormalizedOwner());
        rs = stmt.executeQuery();
        int lsts = rs.next() ? rs.getInt(1) : 0;
        rs.close();
        rs = null;
        stmt.close();
        if (lsts >= maxLst)
          throw new ListsCountExceededException(dl.getOwner());
      }

      stmt = connection.prepareStatement(ADD_DL_SQL);
      stmt.setString(1, dl.getName());
      stmt.setInt(2, dl.getMaxElements());
      final String normalizedOwner = dl.getNormalizedOwner();
      if (dl.isSys() || normalizedOwner == null || normalizedOwner.trim().length() == 0)
        stmt.setNull(3, java.sql.Types.VARCHAR);
      else
        stmt.setString(3, normalizedOwner);
      stmt.executeUpdate();

      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public void deleteDistributionList(String dlname) throws AdminException, ListNotExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(CHECK_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next() || rs.getInt(1) == 0)
        throw new ListNotExistsException(dlname);
      rs.close();
      rs = null;
      stmt.close();

      stmt = connection.prepareStatement(DELETE_MEMBERS_SQL);
      stmt.setString(1, dlname);
      stmt.executeUpdate();
      stmt.close();

      stmt = connection.prepareStatement(DELETE_DL_SUB_SQL);
      stmt.setString(1, dlname);
      stmt.executeUpdate();
      stmt.close();

      stmt = connection.prepareStatement(DELETE_DL_SQL);
      stmt.setString(1, dlname);
      stmt.executeUpdate();

      connection.commit();
    } catch (AdminException exc) {
      throw exc;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
  }

  public DistributionList getDistributionList(String dlname) throws AdminException, ListNotExistsException
  {
    DistributionList dl = null;
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(GET_DL_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next())
        throw new ListNotExistsException(dlname);
      String name = rs.getString(1);
      String owner = rs.getString(2);
      if (rs.wasNull()) owner = null;
      int maxElements = rs.getInt(3);
      dl = new DistributionList(name, owner, maxElements);
      rs.close();
      rs = null;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return dl;
  }

  private List internal_list(String sql_statement, DlFilter filter) throws AdminException
  {
    List list = new ArrayList();
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      stmt = connection.prepareStatement(sql_statement);
      rs = stmt.executeQuery();
      while (rs.next()) {
        String name = rs.getString(1);
        String owner = rs.getString(2);
        if (rs.wasNull()) owner = null;
        int maxElements = rs.getInt(3);
        final DistributionList dl = new DistributionList(name, owner, maxElements);
        if (filter == null || filter.isItemAllowed(dl))
          list.add(dl);
      }
      rs.close();
      rs = null;
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
        throw new AdminException(cexc.getMessage());
      }
    }
    return list;
  }

  public List list() throws AdminException
  {
    return internal_list(LIST_DL_SQL, null);
  }

  public List list(DlFilter filter) throws AdminException
  {
    if (filter.isEmpty())
      return list();

    String where_clause = "";
    if (!filter.isNamesContainsRegexp() && !filter.isOwnersContainsRegexp()) {
      for (int i = 0; i < filter.getNames().length; i++) {
        String name = filter.getNames()[i];
        where_clause += (i == 0 ? "" : " or ") + "(list like '" + name + "%')";
      }
      for (int i = 0; i < filter.getOwners().length; i++) {
        String owner = filter.getOwners()[i];
        where_clause += (i == 0 ? "" : " or ") + "(owner like '" + owner + "%')";
      }
      if (where_clause.length() > 0)
        where_clause = " where " + where_clause;
    }

    return internal_list(LIST_DL_SQL + where_clause, filter);
  }

  public void alterDistributionList(String dlname, int maxElements) throws AdminException, ListNotExistsException
  {
    PreparedStatement stmt = null;
    Connection connection = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();

      // get dl
      stmt = connection.prepareStatement(GET_DL_SQL_FOR_UPDATE);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next())
        throw new ListNotExistsException(dlname);
      //int oldMaxElements = rs.getInt(1);
      String owner = rs.getString(2);
      if (rs.wasNull()) owner = null;
      rs.close();
      rs = null;
      stmt.close();

      //check members count
      stmt = connection.prepareStatement(GET_DLMEMBERS_SQL);
      stmt.setString(1, dlname);
      rs = stmt.executeQuery();
      if (!rs.next())
        throw new ListNotExistsException(dlname);
      int members = rs.getInt(1);
      rs.close();
      rs = null;
      stmt.close();

      if (members > maxElements)
        throw new MembersCountExceededException(dlname, members, maxElements);

      // check owner's max elements
      if (owner != null && owner.trim().length() > 0) {
        stmt = connection.prepareStatement(GET_PRINCIPAL_SQL);
        stmt.setString(1, owner);
        rs = stmt.executeQuery();
        if (!rs.next())
          throw new OwnerNotExistsException(dlname, owner);
        int ownerMaxMembers = rs.getInt(3);
        rs.close();
        rs = null;
        stmt.close();

        if (maxElements > ownerMaxMembers)
          throw new DistListMaxMembersGreaterThanOwnerMaxMembers(dlname, owner, maxElements, ownerMaxMembers);
      }

      ////////////////////////////////////////////////////
      // alter DL
      stmt = connection.prepareStatement(ALT_DL_SQL);
      stmt.setInt(1, maxElements);
      stmt.setString(2, dlname);
      if (stmt.executeUpdate() <= 0)
        throw new ListNotExistsException(dlname);

      stmt.close();

      connection.commit();
    } catch (AdminException exc) {
      try {
        connection.rollback();
      } catch (Throwable e) {
        logger.error("Could not rollback alter DL", e);
      }
      throw exc;
    } catch (Exception exc) {
      logger.error("Exception occured in alter dL", exc);
      try {
        connection.rollback();
      } catch (Throwable e) {
        logger.error("Could not rollback alter DL", e);
      }
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        connection.close();
      } catch (Exception cexc) {
        logger.error("Could not close connection in alter DL", cexc);
        throw new AdminException(cexc.getMessage());
      }
    }
  }

}
