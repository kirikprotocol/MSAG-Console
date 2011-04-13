package ru.sibinco.sponsored.stats.beans;

import ru.sibinco.sponsored.stats.backend.SponsoredRequestPrototype;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class CreateSponsorRequestBean extends StatsBean{

  public static final int RESULT_SAVE = StatsBean.PRIVATE_RESULT;
  public static final int RESULT_CANCEL = StatsBean.PRIVATE_RESULT+1;

  private String mbApply;
  private String mbCancel;

  private String from = "";

  private String till = "";

  private String bonus = "";

  protected int init(List errors) {
    return super.init(errors);
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if(result != RESULT_OK) {
      return result;
    }

    if (mbApply != null) {
      mbApply = null;
      return save();
    } else if (mbCancel != null) {
      mbCancel = null;
      return RESULT_CANCEL;
    }

    return RESULT_OK;
  }


  private int save() {
    SponsoredRequestPrototype prototype = new SponsoredRequestPrototype();

    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");
    try{
      if(from != null && from.length()>0) {
        prototype.setFrom(sdf.parse(from));
      }
      if(till != null && till.length()>0) {
        prototype.setTill(sdf.parse(till));
      }
      if(bonus != null && bonus.length()>0) {
        prototype.setCost(Float.parseFloat(bonus));
      }
    }catch (ParseException e){
      logger.error(e,e);
      return error(e.getMessage());
    }catch (NumberFormatException e){
      logger.error(e,e);
      return error(e.getMessage());
    }
    try {
      sponsoredContext.getStatRequestManager().createRequest(prototype);
    } catch (StatisticsException e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
    return RESULT_SAVE;

  }

  public String getBonus() {
    return bonus;
  }

  public void setBonus(String bonus) {
    this.bonus = bonus;
  }

  public String getFrom() {
    return from;
  }

  public void setFrom(String from) {
    this.from = from;
  }

  public String getTill() {
    return till;
  }

  public void setTill(String till) {
    this.till = till;
  }

  public String getMbApply() {
    return mbApply;
  }

  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }
}
