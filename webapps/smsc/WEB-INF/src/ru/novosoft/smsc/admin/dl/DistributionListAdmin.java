/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 1:04:38 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.exceptions.*;

import java.util.List;

public interface DistributionListAdmin
{
    public void addPrincipal(Principal prc)
        throws AdminException, PrincipalAlreadyExistsException;

    public void addMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberAlreadyExistsException;
    public void deleteMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberNotExistsException;
    public void removeMembers(String dlname)
        throws AdminException, ListNotExistsException;
    public List members(String dlname, String submitter)
        throws AdminException, ListNotExistsException, SubmitterNotExistsException;

    public void grantPosting(String dlname, String address)
        throws AdminException, ListNotExistsException, SubmitterAlreadyExistsException;
    public void revokePosting(String dlname, String address)
        throws AdminException, ListNotExistsException, SubmitterNotExistsException;

    public void addDistributionList(DistributionList dl)
        throws AdminException, ListAlreadyExistsException;
    public void deleteDistributionList(String dlname)
        throws AdminException, ListNotExistsException;

    public boolean checkPermission(String dlname, String address)
        throws AdminException, ListNotExistsException;

    public List list()
        throws AdminException;
}
