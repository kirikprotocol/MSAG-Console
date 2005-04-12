package ru.novosoft.smsc.admin.console.commands.acl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:39:18
 * To change this template use File | Settings | File Templates.
 */
public class AclAlterCommand extends AclGenCommand
{
  public void process(CommandContext ctx)
  {
    if (!isName && !isCache && !isDescription) {
      ctx.setMessage("expecting 'name', 'cache' or 'notes' options. "+
        "Syntax: alter acl <acl_id> [name <name>][notes <description>][<cache (none|full)>]");
      ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
    } else {
      String out = "ACL '"+aclId+"'";
      try
      {
        AclInfo info = ctx.getSmsc().aclGetInfo(aclId);
        if (info == null) {
          ctx.setMessage("Couldn't alter "+out+". ACL not exists");
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } else {
          if (!isName) name = info.getName();
          if (!isCache) cache = info.getCache_mode();
          if (!isDescription) description = info.getDescription();
          ctx.getSmsc().aclUpdateInfo(aclId, name, description, cache);
          ctx.setMessage(out+" altered");
          ctx.setStatus(CommandContext.CMD_OK);
        }
      } catch (AdminException e) {
        ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      }
    }
  }

  public String getId() {
    return "ACL_ALTER";
  }
}
