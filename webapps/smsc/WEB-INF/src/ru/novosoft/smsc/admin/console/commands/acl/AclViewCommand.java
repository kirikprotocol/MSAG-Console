package ru.novosoft.smsc.admin.console.commands.acl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.acl.AclGenCommand;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:39:29
 * To change this template use File | Settings | File Templates.
 */
public class AclViewCommand extends AclGenCommand
{
  private String showAclInfo(AclInfo info)
  {
    String descr = info.getDescription();
    return "ACL "+info.getId()+" '"+info.getName()+"' "+
           "["+((descr != null && descr.length() > 0) ? descr:"-")+"] "+
           "cache "+((info.getCache_mode() == AclGenCommand.CACHE_FULLCACHE) ? "full":"none");
  }

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
        ctx.setMessage(showAclInfo(info));
        ctx.setStatus(CommandContext.CMD_OK);
      }
    } catch (AdminException e) {
      ctx.setMessage("Couldn't view "+out+". Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ACL_VIEW";
  }
}
