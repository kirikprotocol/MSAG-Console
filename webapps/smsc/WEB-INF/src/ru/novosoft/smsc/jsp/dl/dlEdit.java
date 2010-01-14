package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.DistributionList;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalNotExistsException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: 26.02.2003
 * Time: 23:55:08
 */
public class dlEdit extends dlBody {
    private String mbSave = null;
    private String mbCancel = null;
    private DistributionListAdmin admin = null;

    private DistributionList getDistributionList(String name) {
        try {
            List dls = admin.list();
            for (Iterator iterator = dls.iterator(); iterator.hasNext();) {
                DistributionList distributionList = (DistributionList) iterator.next();
                if (distributionList.getName().equalsIgnoreCase(name))
                    return distributionList;
            }
            return null;
        } catch (AdminException e) {
            logger.error("Couldn't list distribution lists", e);
            return null;
        }
    }

    protected int init(List errors) {
        int result = super.init(errors);
        if (result != RESULT_OK)
            return result;

        if (admin == null)
            admin = appContext.getSmsc().getDistributionListAdmin();

        if (name == null || name.length() == 0)
            return error(SMSCErrors.error.dl.dlNotDefined);

//    if (!initialized) {
        DistributionList distributionList = getDistributionList(name);
        if (distributionList == null) {
            logger.error("Unknown distribution list \"" + name + "\"");
            return error(SMSCErrors.error.dl.unknownDL, name);
        }

        initialized = true;
        maxElements = distributionList.getMaxElements();
        system = distributionList.getOwner() == null || distributionList.getOwner().trim().length() == 0;
        owner = system ? "" : distributionList.getOwner();
        try {
            fullSubmittersList = new LinkedList();
            for (Iterator i = admin.submitters(name).iterator(); i.hasNext();) {
                Principal principal = (Principal) i.next();
                fullSubmittersList.add(new Mask(principal.getAddress()).getMask());
            }
        } catch (AdminException e) {
            logger.error("Couldn't get submitters for distribution list \"" + name + "\"", e);
            return error(SMSCErrors.error.dl.couldntGetSubmitters, name);
        }
        try {
            fullMembersList = new LinkedList();
            for (Iterator i = admin.members(name).iterator(); i.hasNext();) {
                String s = (String) i.next();
                fullMembersList.add(new Mask(s).getMask());
            }
        } catch (AdminException e) {
            logger.error("Couldn't get members for distribution list \"" + name + "\"", e);
            return error(SMSCErrors.error.dl.couldntGetMembers, name);
        }
//    }
        return RESULT_OK;
    }

    public int process(HttpServletRequest request) {
        if (mbCancel != null)
            return cancel();

        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (mbSave != null)
            return save();

        return RESULT_OK;
    }

    private int cancel() {
        clear();
        return RESULT_DONE;
    }

    protected void clear() {
        super.clear();
        mbSave = mbCancel = null;
        //admin = null;
    }

    private int save() {
        int result = RESULT_DONE;
        try {
            if ((result = check()) != RESULT_DONE)
                return result;
            if ((result = alterDl()) != RESULT_DONE)
                return result;
            if ((result = applySubmitters()) != RESULT_DONE)
                return result;
            if ((result = applyMembers()) != RESULT_DONE)
                return result;
            journalAppend(SubjectTypes.TYPE_dl, name, Actions.ACTION_MODIFY);
        } catch (AdminException e) {
            logger.error("Couldn't save edited distribution list", e);
            return error(SMSCErrors.error.dl.couldntSave, e);
        }
        clear();
        return result;
    }

    private int check() {
        int result = RESULT_DONE;
        if (fullMembersList.size() > maxElements)
            result = error(SMSCErrors.error.dl.tooMuchMembers);
        if (!system) {
            if (owner == null || owner.trim().length() == 0) {
                result = error(SMSCErrors.error.dl.ownerNotSpecified);
            } else {
                try {
                    Principal principal = admin.getPrincipal(new Mask(owner).getNormalizedMask());
                    if (maxElements > principal.getMaxElements())
                        result = error(SMSCErrors.error.dl.exceedMaxMembersValue, "" + principal.getMaxElements());
                } catch (PrincipalNotExistsException e) {
                    logger.error("Unknown owner \"" + owner + '"');
                    result = error(SMSCErrors.error.dl.unknownOwner, owner);
                } catch (AdminException e) {
                    logger.error("Could not find owner \"" + owner + '"', e);
                    result = error(SMSCErrors.error.dl.couldntFindOwner, owner, e);
                }
            }
        }
        return result;
    }

    private int alterDl() {
        try {
            admin.alterDistributionList(name, maxElements);
        } catch (AdminException e) {
            logger.error("Could not alter distribution list \"" + name + '"', e);
            return error(SMSCErrors.error.dl.couldntAlterDL, name, e);
        }
        return RESULT_DONE;
    }

    private int applySubmitters() throws AdminException, ListNotExistsException, PrincipalNotExistsException {
        int result = RESULT_DONE;
        // apply submitters
        MaskList addrs = new MaskList(fullSubmittersList);
        Set addresses = new HashSet();
        for (Iterator i = addrs.iterator(); i.hasNext();) {
            Mask mask = (Mask) i.next();
            addresses.add(mask.getNormalizedMask());
        }

        for (Iterator i = addrs.iterator(); i.hasNext();) {
            Mask mask = (Mask) i.next();
            if (mask.getQuestionsCount() > 0)
                result = error(SMSCErrors.error.dl.wildcardsNotAllowedInAddress, mask.getMask());
        }
        if (result != RESULT_DONE)
            return result;

        List oldSubmittersPrincipals = admin.submitters(name);
        List oldSubmitters = new LinkedList();
        for (Iterator i = oldSubmittersPrincipals.iterator(); i.hasNext();) {
            Principal principal = (Principal) i.next();
            oldSubmitters.add(principal.getAddress());
        }
        for (Iterator i = oldSubmitters.iterator(); i.hasNext();) {
            String oldSubmitter = (String) i.next();
            Mask m = new Mask(oldSubmitter);
            if (!addresses.contains(m.getNormalizedMask()))
                admin.revokePosting(name, oldSubmitter);
        }
        for (Iterator i = addresses.iterator(); i.hasNext();) {
            String addr = (String) i.next();
            if (!oldSubmitters.contains(addr)) {
                /*    try {
                admin.addPrincipal(new Principal(addr, 0, 0));
              } catch (PrincipalAlreadyExistsException e) {
                // do nothing
              }  */
                admin.grantPosting(name, addr);
            }
        }
        return result;
    }

    private int applyMembers() throws AdminException, ListNotExistsException {
        int result = RESULT_DONE;

        // apply members
        MaskList addrs = new MaskList(fullMembersList);
        Set addresses = new HashSet();
        for (Iterator i = addrs.iterator(); i.hasNext();) {
            Mask mask = (Mask) i.next();
            addresses.add(mask.getNormalizedMask());
        }

        for (Iterator i = addrs.iterator(); i.hasNext();) {
            Mask mask = (Mask) i.next();
            if (mask.getQuestionsCount() > 0)
                result = error(SMSCErrors.error.dl.wildcardsNotAllowedInAddress, mask.getMask());
        }
        if (result != RESULT_DONE)
            return result;

        List oldMembers = admin.members(name);
        for (Iterator i = oldMembers.iterator(); i.hasNext();) {
            String oldMember = (String) i.next();
            Mask m = new Mask(oldMember);
            if (!addresses.contains(m.getNormalizedMask()))
                admin.deleteMember(name, oldMember);
        }
        for (Iterator i = addresses.iterator(); i.hasNext();) {
            String addr = (String) i.next();
            if (!oldMembers.contains(addr))
                admin.addMember(name, addr);
        }
        return result;
    }


    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

    public boolean isCreate() {
        return false;
    }
}
