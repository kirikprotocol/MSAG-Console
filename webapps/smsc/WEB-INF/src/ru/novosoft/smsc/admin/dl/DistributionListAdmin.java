/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 1:04:38 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

public interface DistributionListAdmin
{
    public void addPrincipal(Principal prc) throws AdminException;

    public void addMember(String dlname, String address) throws AdminException;
    public void deleteMember(String dlname, String address) throws AdminException;
    public void removeMembers(String dlname) throws AdminException;
    public List members(String dlname, String submitter) throws AdminException;

    public void grantPosting(String dlname, String address) throws AdminException;
    public void revokePosting(String dlname, String address) throws AdminException;

    public void addDistributionList(DistributionList dl) throws AdminException;
    public void deleteDistributionList(String dlname) throws AdminException;

    public boolean checkPermission(String dlname, String address) throws AdminException;
    public List list() throws AdminException;
}
