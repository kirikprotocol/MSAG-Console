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
            rs.close(); rs = null; stmt.close();

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
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String CHECK_MEMBER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=? AND ADDRESS=?";
    private final static String ADD_MEMBER_SQL =
        "INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (?, ?)";
    public void addMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberAlreadyExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(1, address);
            rs = stmt.executeQuery();
            if (rs.next() && rs.getInt(1) != 0)
              throw new MemberAlreadyExistsException(address, dlname);
            rs.close(); rs = null; stmt.close();

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
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String DELETE_MEMBER_SQL =
        "DELETE FROM DL_MEMBERS WHERE LIST=? AND ADDRESS= ?";
    public void deleteMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberNotExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(1, address);
            rs = stmt.executeQuery();
            if (!rs.next() || rs.getInt(1) == 0)
              throw new MemberNotExistsException(address, dlname);
            rs.close(); rs = null; stmt.close();

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
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String REMOVE_MEMBERS_SQL =
        "DELETE FROM DL_MEMBERS WHERE LIST=?";
    public void removeMembers(String dlname)
        throws AdminException, ListNotExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(REMOVE_MEMBERS_SQL);
            stmt.setString(1, dlname);
            stmt.executeUpdate();
            connection.commit();
            connection.close();
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String GET_MEMBERS_SQL =
        "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=?";
    public List members(String dlname, String submitter)
        throws AdminException, ListNotExistsException, SubmitterNotExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, submitter);
            rs = stmt.executeQuery();
            if (!rs.next() || rs.getInt(1) == 0)
              throw new SubmitterNotExistsException(submitter, dlname);
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(GET_MEMBERS_SQL);
            stmt.setString(1, dlname);
            rs = stmt.executeQuery();
            while (rs.next()) list.add(rs.getString(1));
            rs.close(); rs = null;
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
        return list;
    }

    private final static String CHECK_SUBMITTER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
    private final static String ADD_SUBMITTER_SQL =
        "INSERT INTO DL_SUBMITTERS (LIST, ADDRESS) VALUES (?, ?)";
    public void grantPosting(String dlname, String address)
        throws AdminException, ListNotExistsException, SubmitterAlreadyExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, address);
            rs = stmt.executeQuery();
            if (rs.next() && rs.getInt(1) != 0)
              throw new SubmitterAlreadyExistsException(address, dlname);
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(ADD_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, address);
            stmt.executeUpdate();
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String DELETE_SUBMITTER_SQL =
        "DELETE FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
    public void revokePosting(String dlname, String address)
        throws AdminException, ListNotExistsException, SubmitterNotExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, address);
            rs = stmt.executeQuery();
            if (!rs.next() || rs.getInt(1) == 0)
              throw new SubmitterNotExistsException(address, dlname);
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(DELETE_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, address);
            stmt.executeUpdate();
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String CHECK_DL_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=?";
    private final static String ADD_DL_SQL =
        "INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (?, ?, ?)";
    public void addDistributionList(DistributionList dl)
        throws AdminException, ListAlreadyExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(ADD_DL_SQL);
            stmt.setString(1, dl.getName());
            stmt.setInt   (2, dl.getMaxElements());
            if (dl.isSys() || dl.getOwner() == null)
                 stmt.setNull  (3, java.sql.Types.VARCHAR);
            else stmt.setString(3, dl.getOwner());
            stmt.executeUpdate();
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    private final static String DELETE_DL_SUB_SQL =
        "DELETE FROM DL_SUBMITTERS WHERE LIST=?";
    private final static String DELETE_DL_SQL =
        "DELETE FROM DL_SET WHERE LIST=?";
    public void deleteDistributionList(String dlname)
        throws AdminException, ListNotExistsException
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(REMOVE_MEMBERS_SQL);
            stmt.setString(1, dlname);
            stmt.executeUpdate(); stmt.close();

            stmt = connection.prepareStatement(DELETE_DL_SUB_SQL);
            stmt.setString(1, dlname);
            stmt.executeUpdate(); stmt.close();

            stmt = connection.prepareStatement(DELETE_DL_SQL);
            stmt.setString(1, dlname);
            stmt.executeUpdate();
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
    }

    public boolean checkPermission(String dlname, String address)
        throws AdminException, ListNotExistsException
    {
        boolean permission = false;
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
            rs.close(); rs = null; stmt.close();

            stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
            stmt.setString(1, dlname);
            stmt.setString(2, address);
            rs = stmt.executeQuery();
            permission = rs.next() && (rs.getInt(1) != 0);
            rs.close(); rs = null;
        } catch (AdminException exc) {
            throw exc;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
        return permission;
    }

    private final static String LIST_DL_SQL =
        "SELECT LIST, OWNER, MAX_EL FROM DL_SET";
    public List list()
        throws AdminException
    {
        List list = new ArrayList();
        PreparedStatement stmt = null;
        Connection connection = null;
        ResultSet rs = null;
        try {
            connection = ds.getConnection();
            stmt = connection.prepareStatement(LIST_DL_SQL);
            rs = stmt.executeQuery();
            while (rs.next()) {
                String name  = rs.getString(1);
                String owner = rs.getString(2);
                if (rs.wasNull()) owner = null;
                int maxElements = rs.getInt(3);
                list.add(new DistributionList(name, owner, maxElements));
            }
            rs.close(); rs = null;
        } catch (Exception exc) {
            exc.printStackTrace();
            throw new AdminException(exc.getMessage());
        } finally {
            try { if (stmt != null) stmt.close(); connection.close(); }
            catch (Exception cexc) {
                cexc.printStackTrace();
                throw new AdminException(cexc.getMessage());
            }
        }
        return list;
    }
}
