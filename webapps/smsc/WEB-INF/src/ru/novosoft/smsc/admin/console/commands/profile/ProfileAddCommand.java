/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;

public class ProfileAddCommand extends ProfileGenCommand
{
  private String mask;

  public void setMask(String mask)
  {
    this.mask = mask;
  }

  public void process(CommandContext ctx)
  {
    String out = "Profile for mask '" + mask + "'";

    if (isAliasOptions && !isAliasHide && !isAliasModifiable) {
      ctx.setMessage("expecting alias option(s). " +
                     "Syntax: alias [hide|nohide|substitute] [modifiable|notmodifiable]");
      ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
      return;
    }
    if (isDivertOptions && !isDivert && !isDivertModifiable &&
        !isDivertActiveAbsent && !isDivertActiveBarred && !isDivertActiveBlocked &&
        !isDivertActiveCapacity && !isDivertActiveUnconditional)
    {
      ctx.setMessage("expecting divert option(s). " +
                     "Syntax: divert <string> [absent][barred][blocked][capacity][unconditional] "+
                     "[modifiable|notmodifiable]");
      ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
      return;
    }

    try {
      Mask profileMask = new Mask(mask);
      if (!ctx.getSmsc().isLocaleRegistered(locale))
        throw new Exception("Locale '" + locale + "' is not registered");

      Profile profile = new Profile(profileMask, codepage, ussd7bit, report, locale,
                                    aliasHide, aliasModifiable,
                                    divert, divertActiveUnconditional, divertActiveAbsent,
                                    divertActiveBlocked, divertActiveBarred, divertActiveCapacity,
                                    divertModifiable, udhConcat, translit);

      switch (ctx.getSmsc().profileUpdate(profileMask, profile)) {
        case 1:	//pusUpdated
          ctx.setMessage(out + " was updated");
          ctx.setStatus(CommandContext.CMD_OK);
          break;
        case 2: //pusInserted
          ctx.setMessage(out + " added");
          ctx.setStatus(CommandContext.CMD_OK);
          break;
        case 3: //pusUnchanged
          ctx.setMessage(out + " is identical to default.");
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          break;
        default: // pusError
          ctx.setMessage("Couldn't add " + out + ". Unknown cause");
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          break;
      }
    } catch (Exception e) {
      ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId()
  {
    return "PROFILE_ADD";
  }

}

