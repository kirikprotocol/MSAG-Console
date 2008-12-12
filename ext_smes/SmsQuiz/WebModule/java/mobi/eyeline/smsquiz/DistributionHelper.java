package mobi.eyeline.smsquiz;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 * Date: Dec 12, 2008
 */
public class DistributionHelper {

  private String uid;

  private String timeBegin;

  private String timeEnd;

  private String distrDateEnd;

  private String txmode;

  private String sourceAddress;

  private String[] activeWeekDays = new String[7];

  public static final String SA_FIELD = "sa";
  public static final String TIMEBEGIN_FIELD = "timeBegin";
  public static final String TIMEEND_FIELD = "timeEnd";
  public static final String DATEEND_FIELD = "dateEnd";
  public static final String DAYS_FIELD = "days";
  public static final String TXMODE_FIELD = "txmode";

  public DistributionHelper(String uid){
    this.uid = uid;
  }

  public void processRequest(HttpServletRequest request){
    sourceAddress = (request.getParameter(uid+SA_FIELD)!= null) ? request.getParameter(uid+SA_FIELD) : "";
    timeBegin = (request.getParameter(uid+TIMEBEGIN_FIELD)!=null) ? request.getParameter(uid+TIMEBEGIN_FIELD) : "00:00:00";
    timeEnd = (request.getParameter(uid+TIMEEND_FIELD)!=null) ? request.getParameter(uid+TIMEEND_FIELD) : "23:59:59";
    txmode = (request.getParameter(uid+TXMODE_FIELD)!=null) ? request.getParameter(uid+TXMODE_FIELD) : Boolean.FALSE.toString();
    distrDateEnd = (request.getParameter(uid+DATEEND_FIELD)!=null) ? request.getParameter(uid+DATEEND_FIELD) : "";

    if(request.getParameterValues(uid+DAYS_FIELD) == null) {
      activeWeekDays = new String[]{"Mon","Tue","Wed", "Thu", "Fri", "Sat", "Sun"};
    } else {
      activeWeekDays = request.getParameterValues(uid+DAYS_FIELD);
    }
  }


  public String getTimeBegin() {
    return timeBegin;
  }

  public String getTimeEnd() {
    return timeEnd;
  }

  public boolean isTxmode() {
    return txmode.equals(Boolean.TRUE.toString());
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

  public String[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public String getDistrDateEnd() {
    return distrDateEnd;
  }

  public boolean isWeekDayActive(String weekday) {
    if (activeWeekDays != null) {
      for (int i = 0; i < activeWeekDays.length; i++)
        if ((activeWeekDays[i] != null) && (activeWeekDays[i].equals(weekday))) {
          return true;
        }
    }
    return false;
  }

  public String getUid() {
    return uid;
  }

  public void setTimeBegin(String timeBegin) {
    this.timeBegin = timeBegin;
  }

  public void setTimeEnd(String timeEnd) {
    this.timeEnd = timeEnd;
  }

  public void setDistrDateEnd(String distrDateEnd) {
    this.distrDateEnd = distrDateEnd;
  }

  public void setTxmode(String txmode) {
    this.txmode = txmode;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public void setActiveWeekDays(String[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

}
