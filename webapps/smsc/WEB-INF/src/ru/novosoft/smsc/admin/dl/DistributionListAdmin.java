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
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;

import java.util.List;

public interface DistributionListAdmin
{
    /**
     * Returns list of defined principals (as Principals)
     *
     * @return List of Principals
     * @throws AdminException
     */
    public List principals()
        throws AdminException;
    /**
     * Creates new principal
     *
     * @param prc principal to be created
     * @throws AdminException
     * @throws PrincipalAlreadyExistsException if principal with address already exists
     */
    public void addPrincipal(Principal prc)
        throws AdminException, PrincipalAlreadyExistsException;
    /**
     * Removes principal for specified address
     *
     * @param address principal address to be removed
     * @throws AdminException
     * @throws PrincipalNotExistsException if principal with specified address doesn't exist
     * @throws PrincipalInUseException if principal with specified address is in use as submitter of some list
     */
    public void deletePrincipal(String address)
        throws AdminException, PrincipalNotExistsException, PrincipalInUseException;
    /**
     * Returns principal for specified address
     *
     * @param address principal address to be retreived
     * @return principal for specified address
     * @throws AdminException
     * @throws PrincipalNotExistsException if principal with specified address doesn't exist
     *
     */
    public Principal getPrincipal(String address)
        throws AdminException, PrincipalNotExistsException;

    /**
     * Updates principal's maxList and/or maxElements limits
     *
     * @param prc principal with altered maxList and/or maxElements limits
     * @throws AdminException
     * @throws PrincipalNotExistsException if specified principal doesn't exists
     */
    public void alterPrincipal(Principal prc, boolean altLists, boolean altElements)
        throws AdminException, PrincipalNotExistsException;

    /**
     * Adds new member into specified list
     *
     * @param dlname distribution list name to add to
     * @param address new member address to be added
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     * @throws MemberAlreadyExistsException if specified member is already registered in list
     */
    public void addMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberAlreadyExistsException;
    /**
     * Removes member from specified list
     *
     * @param dlname distribution list name to remove from
     * @param address member address to be removed
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     * @throws MemberNotExistsException if specified member wasn't registered in list
     */
    public void deleteMember(String dlname, String address)
        throws AdminException, ListNotExistsException, MemberNotExistsException;
    /**
     * Returns members' addresses for specified list (as Strings)
     *
     * @param dlname distribution list name
     * @return List of Strings
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     */
    public List members(String dlname)
        throws AdminException, ListNotExistsException;

    /**
     * Allows posting into specified list to submitter
     *
     * @param dlname distribution list name
     * @param submitter submitter address to be granted to post
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     * @throws PrincipalNotExistsException if principal correspending specified submitter not found
     * @throws SubmitterAlreadyExistsException if specified submitter is already granted to post into list
     */
    public void grantPosting(String dlname, String submitter)
        throws AdminException, ListNotExistsException,
               PrincipalNotExistsException, SubmitterAlreadyExistsException;
    /**
     * Denies posting into specified list to submitter
     *
     * @param dlname distribution list name
     * @param submitter submitter address to be denied to post
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     * @throws SubmitterNotExistsException if specified submitter is already denied to post into list
     */
    public void revokePosting(String dlname, String submitter)
        throws AdminException, ListNotExistsException, SubmitterNotExistsException;
    /**
     * Returns submitters' principals for specified list (as Principals)
     *
     * @param dlname distribution list name
     * @return List of Principals
     * @throws AdminException
     * @throws ListNotExistsException if specified list doesn't exist
     */
    public List submitters(String dlname)
        throws AdminException, ListNotExistsException;

    /**
     * Returns list of all registered distribution lists (as DistributionLists)
     *
     * @return List of DistributionList
     * @throws AdminException
     */
    public List list()
        throws AdminException;
	/**
	 * Returns filtered list of registered distribution lists (as DistributionLists)
	 *
	 * @return List of DistributionList
	 * @throws AdminException
	 */
	public List list(DlFilter filter)
			throws AdminException;

    /**
     * Creates new distribution list
     *
     * @param dl distribution list to be created
     * @throws AdminException
     * @throws ListAlreadyExistsException if list with specified name already exist
     */
    public void addDistributionList(DistributionList dl)
        throws AdminException, ListAlreadyExistsException;
    /**
     * Removes distribution list
     *
     * @param dlname distribution list to be removed
     * @throws AdminException
     * @throws ListNotExistsException if list with specified name doesn't exist
     */
    public void deleteDistributionList(String dlname)
        throws AdminException, ListNotExistsException;
    /**
     * Retreives distribution list by name (as DistributionList)
     *
     * @param dlname distribution list to be retreived
     * @return distribution list by name
     * @throws AdminException
     * @throws ListNotExistsException if list with specified name doesn't exist
     */
    public DistributionList getDistributionList(String dlname)
        throws AdminException, ListNotExistsException;

    /**
     * Changes maxElements limit in distribution list
     *
     * @param dlname distribution list name to be updated
     * @throws AdminException
     * @throws ListNotExistsException if list with specified name doesn't exist
     */
    public void alterDistributionList(String dlname, int maxElements)
        throws AdminException, ListNotExistsException;
}
