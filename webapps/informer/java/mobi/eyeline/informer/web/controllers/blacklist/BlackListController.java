package mobi.eyeline.informer.web.controllers.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class BlackListController extends InformerController{

  private String msisdn;

  private boolean init = false;

  private boolean contains;

  private int uploaded = 0;

  private int unrecognized = 0;

  public int getUploaded() {
    return uploaded;
  }

  public int getUnrecognized() {
    return unrecognized;
  }

  public String lookup() {
    if(validateMsisdn()) {
      init = true;
      try {
        contains = getConfiguration().blacklistContains(msisdn);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String add() {
    if(validateMsisdn()) {
      try {
        getConfiguration().addInBlacklist(msisdn, getUserName());
        contains = true;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String remove() {
    if(validateMsisdn()) {
      try {
        getConfiguration().removeFromBlacklist(msisdn, getUserName());
        contains = false;
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public boolean isContains() {
    return contains;
  }

  public void setContains(boolean contains) {
    this.contains = contains;
  }

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) throws AdminException{
    this.msisdn = msisdn;
  }

  private boolean validateMsisdn() {
    if(msisdn == null || !Address.validate(msisdn)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "blacklist.validation.msisdn");
      return false;
    }
    return true;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  @Override
  protected void _uploaded(BufferedReader reader) throws IOException {
    super._uploaded(reader);
    boolean add = getRequestParameter("file_add") != null;
    String line;
    List<String> list = new ArrayList<String>(1000);
    try{
      while((line = reader.readLine()) != null) {
        line = line.trim();
        if(!Address.validate(line)) {
          unrecognized++;
          continue;
        }
        list.add(line);
        if(list.size() == 1000) {
          if(add) {
            getConfiguration().addInBlacklist(list, getUserName());
          }else {
            getConfiguration().removeFromBlacklist(list, getUserName());
          }
          uploaded+=1000;
          list.clear();
        }
      }
      if(!list.isEmpty()) {
        int s = list.size();
        if(add) {
          getConfiguration().addInBlacklist(list, getUserName());
        }else {
          getConfiguration().removeFromBlacklist(list, getUserName());
        }
        uploaded+=s;
      }
    }catch (AdminException e){
      addError(e);
    }
  }
}
