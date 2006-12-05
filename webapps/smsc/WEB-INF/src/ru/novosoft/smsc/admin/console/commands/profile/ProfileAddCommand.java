/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import org.apache.log4j.Category;

public class ProfileAddCommand extends ProfileGenCommand {
  static Category logger = Category.getInstance(ProfileAddCommand.class);

  private String mask;
  int updateResult;

  public void setMask(String mask) {
    this.mask = mask;
  }

  public void process(CommandContext ctx) {
    super.process(ctx);
    String out = "Profile for mask '" + mask + "'";

    if (isAliasOptions && !isAliasHide && !isAliasModifiable) {
      ctx.setMessage("expecting alias option(s). " +
              "Syntax: alias [hide|nohide|substitute] [modifiable|notmodifiable]");
      ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
      return;
    }
    if (isDivertOptions && !isDivert && !isDivertModifiable &&
            !isDivertActiveAbsent && !isDivertActiveBarred && !isDivertActiveBlocked &&
            !isDivertActiveCapacity && !isDivertActiveUnconditional) {
      ctx.setMessage("expecting divert option(s). " +
              "Syntax: divert <string> [absent][barred][blocked][capacity][unconditional] " +
              "[modifiable|notmodifiable]");
      ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
      return;
    }

    try {
      Mask profileMask = new Mask(mask);
      if (!ctx.getSmsc().isLocaleRegistered(locale))
        throw new Exception("Locale '" + locale + "' is not registered");

      //todo profiler-default properties
      if (!isCodepage) {
        codepage = Profile.convertCodepageStringToByte(ctx.getSmsc().getDefaultProfilePropString("DataCoding"));
        ussd7bit = ctx.getSmsc().getDefaultProfilePropBoolean("UssdIn7Bit");
      }
      if (!isReport)
        report = Profile.convertReportOptionsStringToByte(ctx.getSmsc().getDefaultProfilePropString("Report"));
      if (!isAliasHide)
        aliasHide = (ctx.getSmsc().getDefaultProfilePropBoolean("Hide") ? Profile.ALIAS_HIDE_true : Profile.ALIAS_HIDE_true);
      if (!isAliasModifiable) aliasModifiable = ctx.getSmsc().getDefaultProfilePropBoolean("HideModifiable");
      if (!isDivertModifiable) divertModifiable = ctx.getSmsc().getDefaultProfilePropBoolean("DivertModifiable");
      if (!isUdhConcat) udhConcat = ctx.getSmsc().getDefaultProfilePropBoolean("UdhConcat");
      if (!isLocale) locale = ctx.getSmsc().getDefaultProfilePropString("Locale");
      try {
        if(!isInputAccessMask) inputAccessMask = ctx.getSmsc().getDefaultProfilePropInt("AccessMaskIn");
      } catch (Exception e) {
        inputAccessMask = 1;
        logger.warn("Couldn't get default profile AccessMaskIn, using default 1");
      }
      try {
        if(!isOutputAccessMask) outputAccessMask = ctx.getSmsc().getDefaultProfilePropInt("AccessMaskOut");
      } catch (Exception e) {
        outputAccessMask = 1;
        logger.warn("Couldn't get default profile AccessMaskOut, using default 1");
      }
      if (inputAccessBitOff != 0) {
        inputAccessMask = inputAccessMask & (~inputAccessBitOff);
      } else if (inputAccessBitOn != 0) {
        inputAccessMask = inputAccessMask | inputAccessBitOn;
      }
      if (outputAccessBitOff != 0) {
        outputAccessMask = outputAccessMask & (~outputAccessBitOff);
      } else if (outputAccessBitOn != 0) {
        outputAccessMask = outputAccessMask | outputAccessBitOn;
      }

      Profile profile = new Profile(profileMask, codepage, ussd7bit, report, locale,
              aliasHide, aliasModifiable,
              divert, divertActiveUnconditional, divertActiveAbsent,
              divertActiveBlocked, divertActiveBarred, divertActiveCapacity,
              divertModifiable, udhConcat, translit, groupId, inputAccessMask, outputAccessMask, services, (sponsored > 255) ? 255 : (short)sponsored);

      updateResult = ctx.getSmsc().profileUpdate(profileMask, profile);
      switch (updateResult) {
        case 1:    //pusUpdated
          ctx.setMessage(out + " was updated");
          ctx.setStatus(CommandContext.CMD_OK);
          break;
        case 2: //pusInserted
          ctx.setMessage(out + " added");
          ctx.setStatus(CommandContext.CMD_OK);
          break;
        case 3: //pusUnchanged
          ctx.setMessage(out + " is identical to default.");
          ctx.setStatus(CommandContext.CMD_WARNING);
          break;
        default: // pusError
          ctx.setMessage("Couldn't add " + out + ". Unknown cause");
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          break;
      }
    } catch (Exception e) {
      logger.error("Failed to add profile", e);
      ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "PROFILE_ADD";
  }

  public void updateJournalAndStatuses(CommandContext ctx, String userName) {
    byte act = 0;
    switch (updateResult) {
      case 1:
        act = Actions.ACTION_MODIFY;
        break;
      case 2:
        act = Actions.ACTION_ADD;
        break;
      default:
        return;
    }
    journalAppend(ctx, userName, SubjectTypes.TYPE_profile, mask, act);
    ctx.getStatuses().setProfilesChanged(true);
  }
}

