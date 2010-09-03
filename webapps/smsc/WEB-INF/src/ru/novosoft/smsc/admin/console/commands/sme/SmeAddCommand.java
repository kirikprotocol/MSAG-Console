package ru.novosoft.smsc.admin.console.commands.sme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.route.SME;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 16:50:12
 * To change this template use File | Settings | File Templates.
 */
public class SmeAddCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (manager.contains(smeId)) throw new Exception("SME already exists");
            SME sme = new SME(smeId);
            sme.setPriority(priority);
            sme.setType(type);
            sme.setTypeOfNumber(TON);
            sme.setNumberingPlan(NPI);
            sme.setSystemType(systemType);
            sme.setPassword(password);
            sme.setAddrRange(addressRange);
            sme.setSmeN(smeN);
            sme.setWantAlias(wantAlias);
            sme.setTimeout(timeout);
            sme.setReceiptSchemeName(receiptScheme);
            sme.setDisabled(disabled);
            sme.setMode(mode);
            sme.setProclimit(proclimit);
            sme.setSchedlimit(schedlimit);
            sme.setAccessMask(accessMask);
            sme.setForceGsmDataCoding(forceGsmDataCoding);
            sme.setForceSmeReceipt(forceSmeReceipt);
            sme.setCarryOrgDescriptor(carryOrgDescriptor);
            sme.setCarryOrgUserInfo(carryOrgUserInfo);
            sme.setCarrySccpInfo(carrySccpInfo);
            sme.setFillExtraDescriptor(fillExtraDescriptor);
            sme.setSmppPlus(smppPlus);
            sme.setDefaultDcLatin1(defaultDcLatin1);
            manager.add(sme);
//            manager.add(smeId, priority, type, TON, NPI, interfaceVersion, systemType, password,
//                        addressRange, smeN, wantAlias, forceDC, timeout, receiptScheme,
//                        disabled, mode, proclimit, schedlimit, accessMask);
            ctx.setMessage(out+" added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_ADD";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_service, smeId, Actions.ACTION_ADD);
	}
}
