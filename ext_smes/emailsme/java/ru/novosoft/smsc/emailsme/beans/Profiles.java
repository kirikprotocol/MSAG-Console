package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.emailsme.backend.EmailSmeMessages;
import ru.novosoft.smsc.emailsme.backend.EmailSmeService;
import ru.novosoft.smsc.emailsme.backend.Profile;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 16:29:53
 * To change this template use Options | File Templates.
 */
public class Profiles extends SmeBean
{
  public static final int RESULT_EDIT = SmeBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = SmeBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = SmeBean.PRIVATE_RESULT + 2;

  public final static String SEARCH_BY_ADDR = "Address";
  public final static String SEARCH_BY_ID = "ID";

  private String searchString;
  private String searchField;

  private String mbSearch = null;
  private String mbAdd = null;

  private EmailSmeService service;

  private Profile foundProfile = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    service = new EmailSmeService(appContext);

    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbSearch != null && searchString != null) {
      // validate search string
      if (searchField.equals(SEARCH_BY_ADDR) && !Mask.isMaskValid(searchString))
       return error(EmailSmeMessages.errors.invalidAddress);

      // serach profile
      try {
        foundProfile = (searchField.equals(SEARCH_BY_ADDR)) ? service.lookupProfileByAddr(searchString) : service.lookupProfileByUserId(searchString);
        if (foundProfile == null)
          return error(EmailSmeMessages.errors.profileNotFound);
      } catch (AdminException e) {
        return error(EmailSmeMessages.errors.internalError);
      }
      return RESULT_EDIT;
    }

    if (mbAdd != null) return RESULT_ADD;

    return result;
  }

  public Profile getFoundProfile() {
    return foundProfile;
  }

  public boolean isSearchByAddr() {
    return (searchField != null) && (searchField.equals(SEARCH_BY_ADDR));
  }

  public boolean isSearchById() {
    return (searchField != null) && (searchField.equals(SEARCH_BY_ID));
  }

  public String getMbSearch() {
    return mbSearch;
  }

  public void setMbSearch(String mbSearch) {
    this.mbSearch = mbSearch;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getSearchString() {
    return searchString;
  }

  public void setSearchString(String searchString) {
    this.searchString = searchString;
  }

  public String getSearchField() {
    return searchField;
  }

  public void setSearchField(String searchField) {
    this.searchField = searchField;
  }

}
