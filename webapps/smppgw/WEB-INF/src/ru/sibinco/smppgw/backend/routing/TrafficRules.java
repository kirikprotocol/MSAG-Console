package ru.sibinco.smppgw.backend.routing;

import org.apache.log4j.Logger;

import java.util.StringTokenizer;


/**
 * Created by igork Date: 11.05.2004 Time: 19:09:12
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

  public TrafficRules(final String rulesString)
  {
    for (StringTokenizer tokenizer = new StringTokenizer(rulesString, ", ", false); tokenizer.hasMoreTokens();) {
      final String token = tokenizer.nextToken();
      if (token.startsWith("")) {
      } else if ("allow_receive".equals(token)) {
        this.allowReceive = true;
      } else if ("allow_answer".equals(token)) {
        this.allowAnswer = true;
      } else if (token.startsWith("send_limit=")) {
        setSendLimitStr(token.substring("send_limit=".length()));
      } else if ("allow_pssr_resp".equals(token)) {
        this.allowPssrResp = true;
      } else if ("allow_ussr_request".equals(token)) {
        this.allowUssrRequest = true;
      } else if (token.startsWith("ussd_mi_dialog_limit=")) {
        final String limit = token.substring("ussd_mi_dialog_limit=".length());
        if (null != limit && 0 < limit.length())
          this.ussdMiDialogLimit = Long.parseLong(limit);
      } else if ("allow_ussd_dialog_init".equals(token)) {
        this.allowUssdDialogInit = true;
      } else if (token.startsWith("ussd_si_dialog_limit=")) {
        final String limit = token.substring("ussd_si_dialog_limit=".length());
        if (null != limit && 0 < limit.length())
          this.ussdSiDialogLimit = Long.parseLong(limit);
      } else {
        logger.warn("Unknown option in rules string: \"" + token + '"');
      }
    }
  }

  public TrafficRules(final boolean allowReceive, final boolean allowAnswer, final long sendLimit, final byte sendLimitUnit, final boolean allowPssrResp,
                      final boolean allowUssrRequest, final long ussdMiDialogLimit, final boolean allowUssdDialogInit, final long ussdSiDialogLimit)
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

  public TrafficRules(final boolean allowReceive, final boolean allowAnswer, final String sendLimit, final boolean allowPssrResp,
                      final boolean allowUssrRequest, final String ussdMiDialogLimit, final boolean allowUssdDialogInit, final String ussdSiDialogLimit)
  {
    this.allowReceive = allowReceive;
    this.allowAnswer = allowAnswer;
    setSendLimitStr(sendLimit);
    this.allowPssrResp = allowPssrResp;
    this.allowUssrRequest = allowUssrRequest;
    if (null != ussdMiDialogLimit && 0 < ussdMiDialogLimit.length())
      this.ussdMiDialogLimit = Long.parseLong(ussdMiDialogLimit);
    this.allowUssdDialogInit = allowUssdDialogInit;
    if (null != ussdSiDialogLimit && 0 < ussdSiDialogLimit.length())
      this.ussdSiDialogLimit = Long.parseLong(ussdSiDialogLimit);
  }


  private byte getLimitUnit(final String limit)
  {
    if (null != limit && 0 < limit.length()) {
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
          if ('0' > c || '9' < c) logger.warn("Unknown traffic rules limit: " + c + ", day assumed");
          return LIMIT_UNIT_DAY;
      }
    }
    return LIMIT_UNIT_DAY;
  }

  private char getLimitUnit(final byte limit)
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

  private long getLimit(final String limit)
  {
    if (null != limit && 0 < limit.length()) {
      if (Character.isDigit(limit.charAt(limit.length() - 1)))
        return Long.parseLong(limit);
      else
        return Long.parseLong(limit.substring(0, limit.length() - 1));
    } else
      return 0;
  }


  public String getSendLimitStr()
  {
    if (LIMIT_UNIT_UNLIMITED == sendLimitUnit)
      return "U";
    else
      return String.valueOf(sendLimit) + getLimitUnit(sendLimitUnit);
  }

  public void setSendLimitStr(final String limit)
  {
    if ("U".equalsIgnoreCase(limit)) {
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

  public void setAllowReceive(final boolean allowReceive)
  {
    this.allowReceive = allowReceive;
  }

  public boolean isAllowAnswer()
  {
    return allowAnswer;
  }

  public void setAllowAnswer(final boolean allowAnswer)
  {
    this.allowAnswer = allowAnswer;
  }

  public long getSendLimit()
  {
    return sendLimit;
  }

  public void setSendLimit(final long sendLimit)
  {
    this.sendLimit = sendLimit;
  }

  public byte getSendLimitUnit()
  {
    return sendLimitUnit;
  }

  public void setSendLimitUnit(final byte sendLimitUnit)
  {
    this.sendLimitUnit = sendLimitUnit;
  }

  public boolean isAllowPssrResp()
  {
    return allowPssrResp;
  }

  public void setAllowPssrResp(final boolean allowPssrResp)
  {
    this.allowPssrResp = allowPssrResp;
  }

  public boolean isAllowUssrRequest()
  {
    return allowUssrRequest;
  }

  public void setAllowUssrRequest(final boolean allowUssrRequest)
  {
    this.allowUssrRequest = allowUssrRequest;
  }

  public long getUssdMiDialogLimit()
  {
    return ussdMiDialogLimit;
  }

  public void setUssdMiDialogLimit(final long ussdMiDialogLimit)
  {
    this.ussdMiDialogLimit = ussdMiDialogLimit;
  }

  public boolean isAllowUssdDialogInit()
  {
    return allowUssdDialogInit;
  }

  public void setAllowUssdDialogInit(final boolean allowUssdDialogInit)
  {
    this.allowUssdDialogInit = allowUssdDialogInit;
  }

  public long getUssdSiDialogLimit()
  {
    return ussdSiDialogLimit;
  }

  public void setUssdSiDialogLimit(final long ussdSiDialogLimit)
  {
    this.ussdSiDialogLimit = ussdSiDialogLimit;
  }

  public String getText()
  {
    final StringBuffer result = new StringBuffer();
    if (allowReceive) result.append("allow_receive,");
    if (allowAnswer) result.append("allow_answer,");
    result.append("send_limit=").append(getSendLimitStr()).append(',');
    if (allowPssrResp) result.append("allow_pssr_resp,");
    if (allowUssrRequest) result.append("allow_ussr_request,");
    result.append("ussd_mi_dialog_limit=").append(ussdMiDialogLimit).append(',');
    if (allowUssdDialogInit) result.append("allow_ussd_dialog_init,");
    result.append("ussd_si_dialog_limit=").append(ussdSiDialogLimit);
    return result.toString();
  }
}
