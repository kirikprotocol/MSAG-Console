/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */
package ru.novosoft.smsc.jsp.smsc.aliases;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.journal.Action;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.List;


public class AliasesEdit extends SmscBean
{
  protected String mbSave = null;
  protected String mbCancel = null;

  protected String oldAlias = null;
  protected String oldAddress = null;
  protected boolean oldHide = false;

  protected String alias = null;
  protected String address = null;
  protected boolean hide = false;

  protected int init(final List errors)
  {
    final int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    if (null == alias || null == address) {
      alias = address = "";
      hide = false;
    }

    try {
      alias = new Mask(alias).getMask();
    } catch (AdminException e) {
      return error(SMSCErrors.error.aliases.invalidAlias, alias);
    }
    try {
      address = new Mask(address).getMask();
    } catch (AdminException e) {
      return error(SMSCErrors.error.aliases.invalidAddress, address);
    }

    if (null == oldAlias || null == oldAddress) {
      oldAlias = alias;
      oldAddress = address;
      oldHide = false;
    }

    return result;
  }

  public int process(final HttpServletRequest request)
  {
    if (null != mbCancel)
      return RESULT_DONE;

    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (!Mask.isMaskValid(address))
      return error(SMSCErrors.error.aliases.invalidAddress, address);
    if (!Mask.isMaskValid(alias))
      return error(SMSCErrors.error.aliases.invalidAlias, alias);
    if (isHide() && (0 <= address.indexOf('?') || 0 <= alias.indexOf('?')))
      return error(SMSCErrors.error.aliases.HideWithQuestion);
    if (countQuestions(address) != countQuestions(alias))
      return error(SMSCErrors.error.aliases.QuestionCountsNotMathes);

    if (null != mbSave)
      return save();

    return RESULT_OK;
  }

  private int countQuestions(final String str)
  {
    final int start = str.indexOf('?');
    if (0 <= start)
      return str.lastIndexOf('?') - start + 1;
    else
      return 0;
  }

  protected int save()
  {
    final Alias al;
    try {
      al = new Alias(new Mask(address), new Mask(alias), hide);
    } catch (AdminException e) {
      logger.error("Ureachable code reached!", e);
      return error(SMSCErrors.error.aliases.invalidAddress, address);
    }
    final Alias oldAl;
    try {
      oldAl = new Alias(new Mask(oldAddress), new Mask(oldAlias), oldHide);
    } catch (AdminException e) {
      logger.debug("Error in old alias", e);
      return error(SMSCErrors.error.aliases.invalidAddress, oldAddress);
    }

    final boolean aliasChanged = !oldAl.getAlias().equals(al.getAlias());
    final boolean addressChanged = !oldAl.getAddress().equals(al.getAddress());

    if (aliasChanged && smsc.getAliases().contains(al))
      return error(SMSCErrors.error.aliases.alreadyExistsAlias, al.getAlias().getMask());

    if (al.isHide() && smsc.getAliases().isAddressExistsAndHide(al.getAddress(), !addressChanged ? oldAl : null))
      return error(SMSCErrors.error.aliases.alreadyExistsAddress, al.getAddress().getMask());

    try {
      return smsc.getAliases().modify(oldAl, al, new Action(loginedPrincipal.getName(), sessionId))
             ? RESULT_DONE
             : error(SMSCErrors.error.aliases.alreadyExistsAlias, alias);
    } catch (Throwable t) {
      logger.error("Couldn't edit alias \"" + address + "\"-->\"" + alias + "\"", t);
      return error(SMSCErrors.error.aliases.cantEdit, alias);
    }
  }

  /**
   * ************************ properties ********************************
   */

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(final String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(final String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getOldAlias()
  {
    return oldAlias;
  }

  public void setOldAlias(final String oldAlias)
  {
    this.oldAlias = oldAlias;
  }

  public String getOldAddress()
  {
    return oldAddress;
  }

  public void setOldAddress(final String oldAddress)
  {
    this.oldAddress = oldAddress;
  }

  public boolean isOldHide()
  {
    return oldHide;
  }

  public void setOldHide(final boolean oldHide)
  {
    this.oldHide = oldHide;
  }

  public String getAlias()
  {
    return alias;
  }

  public void setAlias(final String alias)
  {
    this.alias = alias;
  }

  public String getAddress()
  {
    return address;
  }

  public void setAddress(final String address)
  {
    this.address = address;
  }

  public boolean isHide()
  {
    return hide;
  }

  public void setHide(final boolean hide)
  {
    this.hide = hide;
  }
}
