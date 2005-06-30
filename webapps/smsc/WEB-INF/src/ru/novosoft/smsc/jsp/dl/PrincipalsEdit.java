package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.dl.exceptions.ListsCountExceededException;
import ru.novosoft.smsc.admin.dl.exceptions.MembersCountExceededForOwnerException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalAlreadyExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalNotExistsException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 22.09.2003
 * Time: 16:51:48
 */
public class PrincipalsEdit extends SmscBean
{
  private String address = null;
  private int max_lst = 0;
  private int max_el = 0;
  private boolean create = false;
  private boolean initialized = false;

  private String mbCancel = null;
  private String mbDone = null;
  private DistributionListAdmin admin = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    admin = appContext.getSmsc().getDistributionListAdmin();
    if (!initialized) {
      if (!create) {
        try {
          ru.novosoft.smsc.admin.dl.Principal principal = admin.getPrincipal(address);
          max_lst = principal.getMaxLists();
          max_el = principal.getMaxElements();
        } catch (PrincipalNotExistsException e) {
          logger.error("Principal \"" + address + "\" not found");
          return error(SMSCErrors.error.dl.principalNotFound, address);
        } catch (AdminException e) {
          logger.error("Cannot load principal \"" + address + "\"", e);
          return error(SMSCErrors.error.dl.cannotLoadPrincipal, address, e);
        }
      }
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbDone != null)
      return done();

    return result;
  }

  private int done()
  {
    Mask addressMask = null;
    try {
      addressMask = new Mask(address);
    } catch (AdminException e) {
      logger.error("Invalid address \"" + address + "\"", e);
      return error(SMSCErrors.error.dl.invalidAddress, address, e);
    }

    ru.novosoft.smsc.admin.dl.Principal principal = new ru.novosoft.smsc.admin.dl.Principal(addressMask.getNormalizedMask(), max_lst, max_el);
    try {
      if (create)
        admin.addPrincipal(principal);
      else
        admin.alterPrincipal(principal, true, true);
    } catch (PrincipalAlreadyExistsException e) {
      logger.error("Principal \"" + address + "\" already exists");
      return error(SMSCErrors.error.dl.principalAlreadyExist, address);
    } catch (PrincipalNotExistsException e) {
      logger.error("Principal \"" + address + "\" not found");
      return error(SMSCErrors.error.dl.principalNotFound, address);
    } catch (ListsCountExceededException e) {
      logger.error("Max lists count (" + max_lst + ") too small for principal \"" + address + "\"");
      return error(SMSCErrors.error.dl.maxListsCountTooSmall);
    } catch (MembersCountExceededForOwnerException e) {
      logger.error("Max members count (" + max_el + ") too small for principal \"" + address + "\"");
      return error(SMSCErrors.error.dl.maxMembersCountTooSmall);
    } catch (AdminException e) {
		logger.error("Could not " + (create ? "create" : "update") + " principal \"" + address + '"', e);
		int i = (create
				? error(SMSCErrors.error.dl.couldntCreatePrincipal, address, e)
				: error(SMSCErrors.error.dl.couldntUpdatePrincipal, address, e)
				);
		return i;
    }
    return RESULT_DONE;
  }

  public String getAddress()
  {
    return address;
  }

  public void setAddress(String address)
  {
    this.address = address;
  }

  public int getMax_lstInt()
  {
    return max_lst;
  }

  public void setMax_lstInt(int max_lst)
  {
    this.max_lst = max_lst;
  }

  public int getMax_elInt()
  {
    return max_el;
  }

  public void setMax_elInt(int max_el)
  {
    this.max_el = max_el;
  }

  public String getMax_lst()
  {
    return String.valueOf(max_lst);
  }

  public void setMax_lst(String max_lst)
  {
    try {
      this.max_lst = Integer.decode(max_lst).intValue();
    } catch (NumberFormatException e) {
      logger.error("Incorrect value \"" + max_lst + "\" for maximum lists", e);
      error(SMSCErrors.error.dl.maxListsIncorrectValue, max_lst, e);
    }
  }

  public String getMax_el()
  {
    return String.valueOf(max_el);
  }

  public void setMax_el(String max_el)
  {
    try {
      this.max_el = Integer.decode(max_el).intValue();
    } catch (NumberFormatException e) {
      logger.error("Incorrect value \"" + max_el + "\" for maximum elements", e);
      error(SMSCErrors.error.dl.maxElementsIncorrectValue, max_el, e);
    }
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }
}
