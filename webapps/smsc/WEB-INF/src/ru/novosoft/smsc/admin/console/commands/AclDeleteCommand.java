package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.07.2004
 * Time: 14:39:09
 * To change this template use File | Settings | File Templates.
 */
public class AclDeleteCommand extends AclGenCommand
{
  public void process(CommandContext ctx)
  {
    String out = "ACL "+aclId;
    try
    {
      ctx.getSmsc().aclRemove(aclId);
      ctx.setMessage(out+" deleted");
      ctx.setStatus(CommandContext.CMD_OK);
    } catch (AdminException e) {
      ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ACL_DELETE";
  }
}
