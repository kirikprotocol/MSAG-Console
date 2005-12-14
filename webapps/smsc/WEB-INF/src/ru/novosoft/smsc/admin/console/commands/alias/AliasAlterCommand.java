/**

 * Created by IntelliJ IDEA.

 * User: makarov

 * Date: Dec 16, 2002

 * Time: 6:54:05 PM

 * To change this template use Options | File Templates.

 */
package ru.novosoft.smsc.admin.console.commands.alias;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Mask;

public class AliasAlterCommand extends CommandClass
{
  private String address = null;
  private String alias = null;
  private boolean hide = false;
  private boolean hideSet = false;
  private boolean aliasChanged = false;
  private String oldAliasMask = null;

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
		ctx.getSmsc().getAliases().remove(alias);
        boolean ok = ctx.getSmsc().getAliases().add(newAlias);
        if (ok) {
			aliasChanged = !smscAlias.getAlias().equals(newAlias.getAlias());
			if (aliasChanged) {oldAliasMask = smscAlias.getAlias().getMask();}
			ctx.setMessage(out+" altered");
			ctx.setStatus(CommandContext.CMD_OK);
        } else {
          ctx.getSmsc().getAliases().add(smscAlias);
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

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		if (aliasChanged) {journalAppend(ctx, userName, SubjectTypes.TYPE_alias, alias, Actions.ACTION_MODIFY, "old alias", oldAliasMask);}
			else journalAppend(ctx, userName, SubjectTypes.TYPE_alias, alias, Actions.ACTION_MODIFY);
		ctx.getStatuses().setAliasesChanged(true);
	}
}



