package mobi.eyeline.welcomesms.beans;

import java.util.List;

import mobi.eyeline.welcomesms.backend.Network;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 */
public class NetworkEditBean extends WelcomeSMSBean{

  private String mbDone;
  private String mbCancel;

  private boolean initialized = false;

  private String countryId;

  private String mcc;

  private String mnc;

  private String name;

  private String id;


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) {
      return result;
    }
    if (!initialized && id != null && id.length() > 0) {
      try{
        Network n = welcomeSMSContext.getNetwork(Integer.parseInt(id));
        if(n == null) {
          logger.error("Network is not found with id: "+id);
          return error("Network is not found");
        }
        countryId = n.getCountryId();
        name = n.getName();
        mcc = Integer.toString(n.getMcc());
        mnc = Integer.toString(n.getMnc());
      }catch(NumberFormatException e){
        logger.error(e,e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) {
      return result;
    }
    if (mbDone != null) result = save();
    else if (mbCancel != null) result = RESULT_DONE;

    return result;
  }

  /** @noinspection EmptyCatchBlock*/
  private int save() {
    try{

      if(mcc == null || (mcc = mcc.trim()).length() == 0) {
        return error("MCC is specified incorrectly");
      }

      if(mnc == null || (mnc = mnc.trim()).length() == 0) {
        return error("MNC is specified incorrectly");
      }

      if(countryId == null || (countryId = countryId.trim()).length() == 0) {
        return error("Country's field is empty");
      }

      if(name == null || (name = name.trim()).length() == 0) {
        return error("Name's field is empty");
      }
      Network network = new Network();
      try{
        network.setMcc(Integer.parseInt(mcc));
        network.setMnc(Integer.parseInt(mnc));
      }catch(NumberFormatException e) {
        return error("MCC or MNC are specified incorrectly");
      }
      network.setName(name);
      network.setCountryId(countryId);
      network.setId(id == null || id.length() == 0 ? null : Integer.valueOf(id));

      welcomeSMSContext.saveNetwork(network);
    }catch(Exception e) {
      return error(e.getMessage());
    }

    return RESULT_DONE;
  }



  public String getMbDone() {
    return mbDone;
  }

  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public String getCountryId() {
    return countryId;
  }

  public void setCountryId(String countryId) {
    this.countryId = countryId;
  }

  public String getMcc() {
    return mcc;
  }

  public void setMcc(String mcc) {
    this.mcc = mcc;
  }

  public String getMnc() {
    return mnc;
  }

  public void setMnc(String mnc) {
    this.mnc = mnc;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }
}
