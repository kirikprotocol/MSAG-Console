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

import javax.sql.DataSource;
import java.util.List;
import java.util.ArrayList;

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
    public void addPrincipal(Principal prc) throws AdminException {

    }

    private final static String CHECK_MEMBER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE ADDRESS= ? AND LIST=?";
    private final static String ADD_MEMBER_SQL =
        "INSERT INTO FROM DL_MEMBERS (ADDRESS, LIST) VALUES (?, ?)";
    public void addMember(String dlname, String address) throws AdminException {
    }

    private final static String DELETE_MEMBER_SQL =
        "DELETE FROM DL_MEMBERS WHERE ADDRESS= ? AND LIST=?";
    public void deleteMember(String dlname, String address) throws AdminException {
    }

    private final static String REMOVE_MEMBERS_SQL =
        "DELETE FROM DL_MEMBERS WHERE AND LIST=?";
    public void removeMembers(String dlname) throws AdminException {
    }

    private final static String GET_MEMBERS_SQL =
        "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=?";
    public List members(String dlname, String submitter) throws AdminException {
        List list = new ArrayList();
        return list;
    }

    private final static String CHECK_SUBMITTER_SQL =
        "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE ADDRESS=? AND LIST=?";
    private final static String ADD_SUBMITTER_SQL =
        "INSERT INTO DL_SUBMITTERS (ADDRESS, LIST) VALUES (?, ?)";
    public void grantPosting(String dlname, String address) throws AdminException {
    }

    private final static String DELETE_SUBMITTER_SQL =
        "DELETE FROM DL_SUBMITTERS WHERE ADDRESS=? AND LIST=?";
    public void revokePosting(String dlname, String address) throws AdminException {
    }

    public void addDistributionList(DistributionList dl) throws AdminException {
    }

    public void deleteDistributionList(String dlname) throws AdminException {
    }

    public boolean checkPermission(String dlname, String address) throws AdminException {
        return false;
    }

    public List list() throws AdminException {
        List list = new ArrayList();
        return list;
    }
}
