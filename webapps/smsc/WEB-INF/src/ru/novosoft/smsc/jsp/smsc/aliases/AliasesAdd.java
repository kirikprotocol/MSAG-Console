package ru.novosoft.smsc.jsp.smsc.aliases;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.journal.Action;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.List;


public class AliasesAdd extends SmscBean
{
  protected String mbSave = null;
  protected String mbCancel = null;

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

    return result;
  }

  public int process(final HttpServletRequest request)
  {
    if (null != mbCancel)
      return RESULT_DONE;

    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

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
    if (!Mask.isMaskValid(address))
      return error(SMSCErrors.error.aliases.invalidAddress, address);
    if (!Mask.isMaskValid(alias))
      return error(SMSCErrors.error.aliases.invalidAlias, alias);
    if (isHide() && (0 <= address.indexOf('?') || 0 <= alias.indexOf('?')))
      return error(SMSCErrors.error.aliases.HideWithQuestion);
    if (countQuestions(address) != countQuestions(alias))
      return error(SMSCErrors.error.aliases.QuestionCountsNotMathes);

    try {
      final Alias newAlias = new Alias(new Mask(address), new Mask(alias), hide);
      if (smsc.getAliases().contains(newAlias))
        return error(SMSCErrors.error.aliases.alreadyExistsAlias, alias);

      return smsc.getAliases().add(newAlias, new Action(loginedPrincipal.getName(), sessionId))
              ? RESULT_DONE
              : error(SMSCErrors.error.aliases.alreadyExistsAddress, alias);
    } catch (Throwable t) {
      logger.error("Couldn't add alias \"" + address + "\"-->\"" + alias + "\"", t);
      return error(SMSCErrors.error.aliases.cantAdd, alias);
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
