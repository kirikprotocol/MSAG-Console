package ru.novosoft.smsc.admin.console.commands.sme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 17:39:33
 * To change this template use File | Settings | File Templates.
 */
public class SmeAlterCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        boolean disconnectOnly = false;
        if (!isAddressRange && !isDisabled && !isMode &&
            !isNPI && !isTON && !isPassword && !isPriority && !isProclimit && !isSchedlimit && !isType &&
            !isReceiptScheme && !isSmeN && !isSystemType && !isTimeout && !isWantAlias && !isAccessMask &&
            !isCarryOrgDescriptor && !isCarryOrgUserInfo && !isCarrySccpInfo && !isFillExtraDescriptor &&
            !isForceSmeReceipt && !isForceGsmDataCoding && !isSmppPlus)
        {
            if (!disconnect) {
                ctx.setMessage("No one alter option specified");
                ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
                return;
            }
            disconnectOnly = true;
        }

        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (!manager.contains(smeId)) throw new Exception("SME not exists");
            if (disconnect && ctx.getSmsc().getSmeStatus(smeId).isConnected()) {
                List toDisconnect = new ArrayList(); toDisconnect.add(smeId);
                manager.disconnectSmes(toDisconnect);
            }
            if (!disconnectOnly) {
                SME sme = manager.get(smeId);
                if (sme == null) throw new Exception("Sme " + smeId + " is not found");
                SME newSme = new SME(smeId);
                newSme.setPriority(isPriority ? priority:sme.getPriority());
                newSme.setType(isType ? type:sme.getType());
                newSme.setTypeOfNumber(isTON ? TON:sme.getTypeOfNumber());
                newSme.setNumberingPlan(isNPI ? NPI:sme.getNumberingPlan());
                newSme.setSystemType(isSystemType ? systemType:sme.getSystemType());
                newSme.setPassword(isPassword ? password:sme.getPassword());
                newSme.setAddrRange(isAddressRange ? addressRange:sme.getAddrRange());
                newSme.setSmeN(isSmeN ? smeN:sme.getSmeN());
                newSme.setWantAlias(isWantAlias ? wantAlias:sme.isWantAlias());
                newSme.setTimeout(isTimeout ? timeout:sme.getTimeout());
                newSme.setReceiptSchemeName(isReceiptScheme ? receiptScheme:sme.getReceiptSchemeName());
                newSme.setDisabled(isDisabled ? disabled:sme.isDisabled());
                newSme.setMode(isMode ? mode:sme.getMode());
                newSme.setProclimit(isProclimit ? proclimit:sme.getProclimit());
                newSme.setSchedlimit(isSchedlimit ? schedlimit:sme.getSchedlimit());
                newSme.setAccessMask(isAccessMask ? accessMask:sme.getAccessMask());
                newSme.setCarryOrgDescriptor(isCarryOrgDescriptor ? carryOrgDescriptor : sme.isCarryOrgDescriptor());
                newSme.setCarryOrgUserInfo(isCarryOrgUserInfo ? carryOrgUserInfo : sme.isCarryOrgUserInfo());
                newSme.setCarrySccpInfo(isCarrySccpInfo ? carrySccpInfo : sme.isCarrySccpInfo());
                newSme.setFillExtraDescriptor(isFillExtraDescriptor ? fillExtraDescriptor : sme.isFillExtraDescriptor());
                newSme.setForceSmeReceipt(isForceSmeReceipt ? forceSmeReceipt : sme.isForceSmeReceipt());
                newSme.setForceGsmDataCoding(isForceGsmDataCoding ? forceGsmDataCoding : sme.isForceGsmDataCoding());
                newSme.setSmppPlus(isSmppPlus ? smppPlus : sme.isSmppPlus());
                newSme.setDefaultDcLatin1(isDefaultDcLatin1 ? defaultDcLatin1 : sme.isDefaultDcLatin1());

//                SME newSme = new SME(smeId, isPriority ? priority:sme.getPriority(), isType ? type:sme.getType(),
//                        isTON ? TON:sme.getTypeOfNumber(), isNPI ? NPI:sme.getNumberingPlan(),
//                        isInterfaceVersion ? interfaceVersion:sme.getInterfaceVersion(),
//                        isSystemType ? systemType:sme.getSystemType(), isPassword ? password:sme.getPassword(),
//                        isAddressRange ? addressRange:sme.getAddrRange(), isSmeN ? smeN:sme.getSmeN(),
//                        isWantAlias ? wantAlias:sme.isWantAlias(), isForceDC ? forceDC:sme.isForceDC(),
//                        isTimeout ? timeout:sme.getTimeout(), isReceiptScheme ? receiptScheme:sme.getReceiptSchemeName(),
//                        isDisabled ? disabled:sme.isDisabled(), isMode ? mode:sme.getMode(),
//                        isProclimit ? proclimit:sme.getProclimit(), isSchedlimit ? schedlimit:sme.getSchedlimit(),
//                        isAccessMask ? accessMask:sme.getAccessMask());
                manager.update(newSme);
            }
            ctx.setMessage(out+" altered");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_ALTER";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_service, smeId, Actions.ACTION_MODIFY);
	}
}
