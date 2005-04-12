/**

 * Created by IntelliJ IDEA.

 * User: makarov

 * Date: Dec 16, 2002

 * Time: 6:54:05 PM

 * To change this template use Options | File Templates.

 */
package ru.novosoft.smsc.admin.console.commands.alias;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.Action;

public class AliasAlterCommand implements Command
{
  private String address = null;
  private String alias = null;
  private boolean hide = false;
  private boolean hideSet = false;

  public void setAddress(String address) {
    this.address = address;
  }
  public void setAlias(String alias) {
    this.alias = alias;
  }
  public void setHide(boolean hide) {
    this.hide = hide; hideSet = true;
  }

  public void process(CommandContext ctx)
  {
    String out = "Alias '"+alias+"'";
    Alias smscAlias = ctx.getSmsc().getAliases().get(alias);
    if (smscAlias != null) {
      try {
        Alias newAlias = new Alias(new Mask(address), new Mask(alias), (hideSet) ? hide:smscAlias.isHide());
        boolean ok = ctx.getSmsc().getAliases().modify(new Alias(new Mask(alias), new Mask(alias), false), newAlias, new Action("console user", Constants.CONSOLE_SESSION_ID)); //todo: pass console user name to method
        if (ok) {
          ctx.setMessage(out+" altered");
          ctx.setStatus(CommandContext.CMD_OK);
        } else {
          ctx.getSmsc().getAliases().add(smscAlias, new Action("console user", Constants.CONSOLE_SESSION_ID)); //todo: pass console user name to method
          ctx.setMessage(out+" already exists or has equivalent");
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
      } catch (Exception e) {
        e.printStackTrace();
        ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      }
    } else {
      ctx.setMessage(out+" not found");
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "ALIAS_ALTER";
  }
}



