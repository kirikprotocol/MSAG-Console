package ru.sibinco.smppgw.backend.routing;

import org.apache.log4j.Logger;

import java.util.StringTokenizer;


/**
 * Created by igork
 * Date: 11.05.2004
 * Time: 19:09:12
 */
public class TrafficRules
{
  private static final int LIMIT_UNIT_UNLIMITED = 1;
  private static final int LIMIT_UNIT_HOUR = 2;
  private static final int LIMIT_UNIT_DAY = 3;
  private static final int LIMIT_UNIT_WEEK = 4;
  private static final int LIMIT_UNIT_MONTH = 5;

  private boolean allowReceive = false;
  private boolean allowAnswer = false;
  private long sendLimit = 0;//=U
  private byte sendLimitUnit = LIMIT_UNIT_UNLIMITED;
  private boolean allowPssrResp = false;
  private boolean allowUssrRequest = false;
  private long ussdMiDialogLimit = 0;
  private boolean allowUssdDialogInit = false;
  private long ussdSiDialogLimit = 0;

  private Logger logger = Logger.getLogger(this.getClass());

  public TrafficRules(String rulesString)
  {
    for (StringTokenizer tokenizer = new StringTokenizer(rulesString, ", ", false); tokenizer.hasMoreTokens();) {
      String token = tokenizer.nextToken();
      if (token.startsWith("")) {
      } else if (token.equals("allow_receive")) {
        this.allowReceive = true;
      } else if (token.equals("allow_answer")) {
        this.allowAnswer = true;
      } else if (token.startsWith("send_limit=")) {
        setSendLimitStr(token.substring("send_limit=".length()));
      } else if (token.equals("allow_pssr_resp")) {
        this.allowPssrResp = true;
      } else if (token.equals("allow_ussr_request")) {
        this.allowUssrRequest = true;
      } else if (token.startsWith("ussd_mi_dialog_limit=")) {
        final String limit = token.substring("ussd_mi_dialog_limit=".length());
        if (limit != null && limit.length() > 0)
          this.ussdMiDialogLimit = Long.parseLong(limit);
      } else if (token.equals("allow_ussd_dialog_init")) {
        this.allowUssdDialogInit = true;
      } else if (token.startsWith("ussd_si_dialog_limit=")) {
        final String limit = token.substring("ussd_si_dialog_limit=".length());
        if (limit != null && limit.length() > 0)
          this.ussdSiDialogLimit = Long.parseLong(limit);
      } else {
        logger.warn("Unknown option in rules string: \"" + token + '"');
      }
    }
  }

  public TrafficRules(boolean allowReceive, boolean allowAnswer, long sendLimit, byte sendLimitUnit, boolean allowPssrResp, boolean allowUssrRequest, long ussdMiDialogLimit, boolean allowUssdDialogInit, long ussdSiDialogLimit)
  {
    this.allowReceive = allowReceive;
    this.allowAnswer = allowAnswer;
    this.sendLimit = sendLimit;
    this.sendLimitUnit = sendLimitUnit;
    this.allowPssrResp = allowPssrResp;
    this.allowUssrRequest = allowUssrRequest;
    this.ussdMiDialogLimit = ussdMiDialogLimit;
    this.allowUssdDialogInit = allowUssdDialogInit;
    this.ussdSiDialogLimit = ussdSiDialogLimit;
  }

  public TrafficRules(boolean allowReceive, boolean allowAnswer, String sendLimit, boolean allowPssrResp, boolean allowUssrRequest, String ussdMiDialogLimit, boolean allowUssdDialogInit, String ussdSiDialogLimit)
  {
    this.allowReceive = allowReceive;
    this.allowAnswer = allowAnswer;
    setSendLimitStr(sendLimit);
    this.allowPssrResp = allowPssrResp;
    this.allowUssrRequest = allowUssrRequest;
    if (ussdMiDialogLimit != null && ussdMiDialogLimit.length() > 0)
      this.ussdMiDialogLimit = Long.parseLong(ussdMiDialogLimit);
    this.allowUssdDialogInit = allowUssdDialogInit;
    if (ussdSiDialogLimit != null && ussdSiDialogLimit.length() > 0)
      this.ussdSiDialogLimit = Long.parseLong(ussdSiDialogLimit);
  }


  private byte getLimitUnit(String limit)
  {
    if (limit != null && limit.length() > 0) {
      final char c = limit.charAt(limit.length() - 1);
      switch (Character.toUpperCase(c)) {
        case 'H':
          return LIMIT_UNIT_HOUR;
        case 'D':
          return LIMIT_UNIT_DAY;
        case 'W':
          return LIMIT_UNIT_WEEK;
        case 'M':
          return LIMIT_UNIT_MONTH;
        default:
          if (c < '0' || c > '9') logger.warn("Unknown traffic rules limit: " + c + ", day assumed");
          return LIMIT_UNIT_DAY;
      }
    }
    return LIMIT_UNIT_DAY;
  }

  private char getLimitUnit(byte limit)
  {
    switch (limit) {
      case LIMIT_UNIT_HOUR:
        return 'H';
      case LIMIT_UNIT_DAY:
        return 'D';
      case LIMIT_UNIT_WEEK:
        return 'W';
      case LIMIT_UNIT_MONTH:
        return 'M';
      default:
        logger.warn("Unknown traffic rules limit: " + limit + ", day assumed");
        return 'D';
    }
  }

  private long getLimit(String limit)
  {
    if (limit != null && limit.length() > 0) {
      if (Character.isDigit(limit.charAt(limit.length() - 1)))
        return Long.parseLong(limit);
      else
        return Long.parseLong(limit.substring(0, limit.length() - 1));
    } else
      return 0;
  }


  public String getSendLimitStr()
  {
    if (sendLimitUnit == LIMIT_UNIT_UNLIMITED)
      return "U";
    else
      return String.valueOf(sendLimit) + getLimitUnit(sendLimitUnit);
  }

  public void setSendLimitStr(String limit)
  {
    if (limit.equalsIgnoreCase("U")) {
      this.sendLimit = 0;
      this.sendLimitUnit = LIMIT_UNIT_UNLIMITED;
    } else {
      this.sendLimit = getLimit(limit);
      this.sendLimitUnit = getLimitUnit(limit);
    }
  }


  public boolean isAllowReceive()
  {
    return allowReceive;
  }

  public void setAllowReceive(boolean allowReceive)
  {
    this.allowReceive = allowReceive;
  }

  public boolean isAllowAnswer()
  {
    return allowAnswer;
  }

  public void setAllowAnswer(boolean allowAnswer)
  {
    this.allowAnswer = allowAnswer;
  }

  public long getSendLimit()
  {
    return sendLimit;
  }

  public void setSendLimit(long sendLimit)
  {
    this.sendLimit = sendLimit;
  }

  public byte getSendLimitUnit()
  {
    return sendLimitUnit;
  }

  public void setSendLimitUnit(byte sendLimitUnit)
  {
    this.sendLimitUnit = sendLimitUnit;
  }

  public boolean isAllowPssrResp()
  {
    return allowPssrResp;
  }

  public void setAllowPssrResp(boolean allowPssrResp)
  {
    this.allowPssrResp = allowPssrResp;
  }

  public boolean isAllowUssrRequest()
  {
    return allowUssrRequest;
  }

  public void setAllowUssrRequest(boolean allowUssrRequest)
  {
    this.allowUssrRequest = allowUssrRequest;
  }

  public long getUssdMiDialogLimit()
  {
    return ussdMiDialogLimit;
  }

  public void setUssdMiDialogLimit(long ussdMiDialogLimit)
  {
    this.ussdMiDialogLimit = ussdMiDialogLimit;
  }

  public boolean isAllowUssdDialogInit()
  {
    return allowUssdDialogInit;
  }

  public void setAllowUssdDialogInit(boolean allowUssdDialogInit)
  {
    this.allowUssdDialogInit = allowUssdDialogInit;
  }

  public long getUssdSiDialogLimit()
  {
    return ussdSiDialogLimit;
  }

  public void setUssdSiDialogLimit(long ussdSiDialogLimit)
  {
    this.ussdSiDialogLimit = ussdSiDialogLimit;
  }
}
