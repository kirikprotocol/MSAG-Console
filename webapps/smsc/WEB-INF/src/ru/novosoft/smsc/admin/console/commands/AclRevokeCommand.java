package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.ArrayList;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:39:56
 * To change this template use File | Settings | File Templates.
 */
public class AclRevokeCommand extends AclGenCommand
{
  public void process(CommandContext ctx)
  {
    String out = "ACL "+aclId;
    try
    {
      AclInfo info = ctx.getSmsc().aclGetInfo(aclId);
      if (info == null) {
        ctx.setMessage(out+" not found");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      } else {
        List list = new ArrayList(); list.add(address);
        ctx.getSmsc().aclRemoveAddresses(aclId, list);
        ctx.setMessage("Access revoked for '"+address+"' to "+out);
        ctx.setStatus(CommandContext.CMD_OK);
      }
    } catch (AdminException e) {
      ctx.setMessage("Couldn't revoke access to "+out+" for address '"+
                     address+"'. Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ACL_REVOKE";
  }
}
