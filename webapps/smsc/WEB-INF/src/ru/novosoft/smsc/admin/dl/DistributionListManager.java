/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 17, 2003
 * Time: 1:43:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.exceptions.*;

import javax.sql.DataSource;
import java.util.List;
import java.util.ArrayList;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

public class DistributionListManager implements DistributionListAdmin
{
    private Smsc smsc;
    private DataSource ds;

    public DistributionListManager(Smsc smsc, DataSource ds) {
        this.smsc = smsc; this.ds = ds;
    }

    private final static String CHECK_PRINCIPAL_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_PRINCIPALS WHERE ADDRESS=?";
    private final static String ADD_PRINCIPAL_SQL =
        "INSERT INTO DL_PRINCIPALS (ADDRESS, MAX_LST, MAX_EL) VALUES (?, ?, ?)";
    public void addPrincipal(Principal prc)
        throws AdminException, PrincipalAlreadyExistsException
    {
        Connection connection = null;
        try
        {
          connection = ds.getConnection();
          PreparedStatement stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
          stmt.setString(1, prc.getAddress());
          ResultSet rs = stmt.executeQuery();
          if (!rs.next() || rs.getInt(1) != 0)
            throw new PrincipalAlreadyExistsException(prc);

          stmt = connection.prepareStatement(ADD_PRINCIPAL_SQL);
          stmt.setString(1, prc.getAddress());
          stmt.setInt(2, prc.getMaxLists());
          stmt.setInt(3, prc.getMaxElements());
          stmt.executeUpdate();
          connection.commit();
          connection.close();
        }
        catch (Exception exc)
        {
          exc.printStackTrace();
          try {
              if (connection != null) {
                connection.rollback();
                connection.close();
              }
          } catch (Exception cexc) { cexc.printStackTrace(); }
          throw new AdminException(exc.getMessage());
        }
    }

    private final static String CHECK_MEMBER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=? AND ADDRESS=?";
    private final static String ADD_MEMBER_SQL =
        "INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (?, ?)";
    public void addMember(String dlname, String address)
        throws AdminException, MemberAlreadyExistsException
    {
    }

    private final static String DELETE_MEMBER_SQL =
        "DELETE FROM DL_MEMBERS WHERE LIST=? AND ADDRESS= ?";
    public void deleteMember(String dlname, String address)
        throws AdminException, MemberNotExistsException
    {
    }

    private final static String REMOVE_MEMBERS_SQL =
        "DELETE FROM DL_MEMBERS WHERE LIST=?";
    public void removeMembers(String dlname)
        throws AdminException, ListNotExistsException
    {
    }

    private final static String GET_MEMBERS_SQL =
        "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=?";
    public List members(String dlname, String submitter)
        throws AdminException, ListNotExistsException, IllegalSubmitterException
    {
        List list = new ArrayList();
        // get members first than check submitter
        return list;
    }

    private final static String CHECK_SUBMITTER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE ADDRESS=? AND LIST=?";
    private final static String ADD_SUBMITTER_SQL =
        "INSERT INTO DL_SUBMITTERS (ADDRESS, LIST) VALUES (?, ?)";
    public void grantPosting(String dlname, String address)
        throws AdminException, ListNotExistsException
    {
    }

    private final static String DELETE_SUBMITTER_SQL =
        "DELETE FROM DL_SUBMITTERS WHERE ADDRESS=? AND LIST=?";
    public void revokePosting(String dlname, String address)
        throws AdminException, ListNotExistsException
    {
    }

    private final static String CHECK_DL_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=?";
    private final static String ADD_DL_SQL =
        "INSERT INTO DL_SET (LIST, SYS, MAX_EL, OWNER) VALUES (?, ?, ?, ?)";
    public void addDistributionList(DistributionList dl)
        throws AdminException, ListAlreadyExistsException
    {
    }

    private final static String DELETE_DL_SUB_SQL =
        "DELETE FROM DL_SUBMITTERS WHERE LIST=?";
    private final static String DELETE_DL_MEM_SQL =
        "DELETE FROM DL_MEMBERS WHERE LIST=?";
    private final static String DELETE_DL_SQL =
        "DELETE FROM DL_SET WHERE LIST=?";
    public void deleteDistributionList(String dlname)
        throws AdminException, ListNotExistsException
    {
    }

    private final static String CHECK_PERM_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
    public boolean checkPermission(String dlname, String address)
        throws AdminException, ListNotExistsException
    {
        return false;
    }

    private final static String LIST_DL_SQL =
        "SELECT LIST, SYS, MAX_EL, OWNER FROM DL_SET";
    public List list()
        throws AdminException
    {
        List list = new ArrayList();
        return list;
    }
}
