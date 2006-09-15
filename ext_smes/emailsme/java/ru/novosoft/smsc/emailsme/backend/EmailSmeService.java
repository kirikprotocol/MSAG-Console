package ru.novosoft.smsc.emailsme.backend;

import antlr.RecognitionException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.console.commands.emailsme.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;

/**
 * User: artem
 * Date: 08.09.2006
 */

public class EmailSmeService  {

  private final SMSCAppContext context;

  public EmailSmeService(SMSCAppContext context) {
    this.context = context;
  }

  public final boolean addProfile(Profile profile) {
    if (profile == null)
      return false;

    final EmailSmeAddCommand addCommand = new EmailSmeAddCommand();
    try {
      final Mask mask = new Mask(profile.getAddr());

      addCommand.setAddress(mask.getMaskSimple());
      addCommand.setUserName((profile.getUserid() == null) ? "" : profile.getUserid());
      addCommand.setRealName(profile.getRealName());
      addCommand.setLimitType(EmailSmeContext.LIMITCHAR_DAY_CHAR);
      addCommand.setLimitValue(profile.getDayLimit());
      addCommand.setForwardEmail((profile.getForward() == null) ? "" : profile.getForward());
      addCommand.setNpi(mask.getNpi());
      addCommand.setTon(mask.getTone());
      addCommand.process(context);

    } catch (AdminException e) {
      return false;
    } catch (RecognitionException e) {
      return false;
    }

    return true;
  }

  public final boolean deleteProfile(Profile profile) {
    if (profile == null)
      return false;

    final EmailSmeDelCommand delCommand = new EmailSmeDelCommand();
    try {
      final Mask mask = new Mask(profile.getAddr());

      delCommand.setAddress(mask.getMaskSimple());
      delCommand.setNpi(mask.getNpi());
      delCommand.setTon(mask.getTone());
      delCommand.process(context);

      return true;
    } catch (AdminException e) {
      return false;
    }
  }

  public final boolean updateProfile(Profile profile) {
    return addProfile(profile);
  }

  public final Profile lookupProfileByAddr(String addr) throws AdminException {
    if (addr == null)
      throw new AdminException("Address id is null");

    final EmailSmeLookupByAddressCommand lookupByAddressCommand = new EmailSmeLookupByAddressCommand();

    final Mask mask = new Mask(addr);
    lookupByAddressCommand.setAddress(mask.getMaskSimple());
    lookupByAddressCommand.setTon(mask.getTone());
    lookupByAddressCommand.setNpi(mask.getNpi());

    final LookupResult result = lookupByAddressCommand.process(context);

    return (result != null) ? new Profile(new Mask(result.getTon(), result.getNpi(),result.getAddr()).getMask(), result.getUserName(), result.getLimitValue(), result.getForwardEmail(), result.getRealName()) : null;

  }

  public final Profile lookupProfileByUserId(String userId) throws AdminException {
    if (userId == null)
      throw new AdminException("User id is null");

    final EmailSmeLookupByUserIdCommand lookupCommand = new EmailSmeLookupByUserIdCommand();
    lookupCommand.setUserId(userId);

    final LookupResult result = lookupCommand.process(context);
    return (result != null) ? new Profile(new Mask(result.getTon(), result.getNpi(),result.getAddr()).getMask(), result.getUserName(), result.getLimitValue(), result.getForwardEmail(), result.getRealName()) : null;
  }
}
