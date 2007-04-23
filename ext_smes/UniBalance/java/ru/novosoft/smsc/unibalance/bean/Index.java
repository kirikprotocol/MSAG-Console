package ru.novosoft.smsc.unibalance.bean;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.unibalance.backend.SmeContext;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

/**
 * User: artem
 * Date: 10.01.2007
 */
public class Index extends SmeBean {

  public static final String CURRENCY_PREFIX = "cur_pref";
  public static final String CURRENCY_CODE_PREFIX = "cur_code_pref";
  public static final String CURRENCY_NAME_PREFIX = "cur_name_pref";

  private String mbStart = null;
  private String mbStop = null;
  private String mbSave = null;
  private String mbCancel = null;

  private String balanceResponse = null;
  private String balanceNegativeResponse = null;
  private String balanceWithAccumulatorResponse = null;
  private String balanceWithAccumulatorNegativeResponse = null;
  private String balanceWaitForSmsResponse = null;
  private String balanceError = null;
  private String balanceCurrencyDefault = null;
  private String balanceBannerAdd = null;
  private ArrayList balanceCurrencyDescriptions = new ArrayList();

  private Properties config = null;

  protected int init(final List errors) {
    int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    config = getConfig();

    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbStart != null)
      result = processStart();
    else if (mbStop != null)
      result = processStop();
    else if (mbSave != null)
      result = processSave(request);
    else if (mbCancel != null)
      result = processCancel();

    configToProps();

    return result;
  }

  private int processStart() {
    mbStart = null;
    try {
      appContext.getHostsManager().startService(SmeContext.SME_ID);
    } catch (AdminException e) {
      return error("Could not start unibalance SME", e);
    }
    return RESULT_OK;
  }

  private int processStop() {
    mbStop = null;
    try {
      appContext.getHostsManager().shutdownService(SmeContext.SME_ID);
    } catch (AdminException e) {
      return error("Could not stop unibalance SME", e);
    }
    return RESULT_OK;
  }

  private int processSave(HttpServletRequest request) {
    mbSave = null;
    readBalanceCurrencyDescriptions(request);
    propsToConfig();
    return storeConfig();
  }

  private int processCancel() {
    mbCancel = null;
    return RESULT_OK;
  }

  public String getCurrencySectionParamName(int num) {
    return CURRENCY_PREFIX + String.valueOf(num);
  }

  public String getCurrencyCodeParamName(int num) {
    return getCurrencySectionParamName(num) + CURRENCY_CODE_PREFIX;
  }

  public String getCurrencyNameParamName(int num) {
    return getCurrencySectionParamName(num) + CURRENCY_NAME_PREFIX;
  }

  private void readBalanceCurrencyDescriptions(HttpServletRequest request) {
    balanceCurrencyDescriptions.clear();

    int count = Integer.parseInt(request.getParameter("tableCounter"));
    for (int i=1; i<= count; i++) {
      final String currencyCode = request.getParameter(getCurrencyCodeParamName(i));
      final String currencyName = request.getParameter(getCurrencyNameParamName(i));
      if (currencyCode != null)
        balanceCurrencyDescriptions.add(new BalanceCurrencyDescription(currencyCode, currencyName));
    }
  }

  private boolean configHasCurrencyWithNumber(int number) {
    return config != null && (config.getProperty("balance.currency." + number + ".code") != null || config.getProperty("balance.currency." + number + ".name") != null);
  }

  private void configToProps() {
    balanceResponse = config.getProperty("balance.response.pattern");
    balanceWaitForSmsResponse = config.getProperty("balance.wait.for.sms.response.pattern");
    balanceError = config.getProperty("balance.error.pattern");
    balanceCurrencyDefault = config.getProperty("balance.currency.default");
    balanceBannerAdd = config.getProperty("balance.banner.add.pattern");
    balanceNegativeResponse = config.getProperty("balance.negative.response.pattern");
    if (balanceNegativeResponse == null)
      balanceNegativeResponse = "";
    balanceWithAccumulatorResponse = config.getProperty("balance.with.accumulator.response.pattern");
    balanceWithAccumulatorNegativeResponse = config.getProperty("balance.with.accumulator.negative.response.pattern");
    if (balanceWithAccumulatorNegativeResponse == null)         
      balanceWithAccumulatorNegativeResponse = "";

    balanceCurrencyDescriptions.clear();
    int i=1;
    while (configHasCurrencyWithNumber(i)) {
      final String currencyCode = config.getProperty("balance.currency." + i + ".code");
      final String currencyName = config.getProperty("balance.currency." + i + ".name");
      balanceCurrencyDescriptions.add(new BalanceCurrencyDescription(currencyCode, currencyName));
      i++;
    }
  }

  private void propsToConfig() {
    config.clear();
    config.setProperty("balance.response.pattern", balanceResponse == null ? "" : balanceResponse);
    config.setProperty("balance.wait.for.sms.response.pattern", balanceWaitForSmsResponse == null ? "" : balanceWaitForSmsResponse);
    config.setProperty("balance.error.pattern", balanceError == null ? "" : balanceError);
    config.setProperty("balance.currency.default", balanceCurrencyDefault == null ? "" : balanceCurrencyDefault);
    config.setProperty("balance.banner.add.pattern", balanceBannerAdd == null ? "" : balanceBannerAdd);

    if (balanceNegativeResponse != null && balanceNegativeResponse.length() > 0)
      config.setProperty("balance.negative.response.pattern", balanceNegativeResponse);

    config.setProperty("balance.with.accumulator.response.pattern", balanceWithAccumulatorResponse == null ? "" : balanceWithAccumulatorResponse);

    if (balanceWithAccumulatorNegativeResponse != null && balanceWithAccumulatorNegativeResponse.length() > 0)
      config.setProperty("balance.with.accumulator.negative.response.pattern", balanceWithAccumulatorNegativeResponse);

    for (int i=0; i < balanceCurrencyDescriptions.size(); i++) {
      final BalanceCurrencyDescription desc = (BalanceCurrencyDescription)balanceCurrencyDescriptions.get(i);
      config.setProperty("balance.currency." + String.valueOf(i+1) + ".code", desc.getCurrencyCode());
      config.setProperty("balance.currency." + String.valueOf(i+1) + ".name", desc.getCurrencyName());
    }
  }

  private int storeConfig() {
    if (config == null)
      return error("Can't store unibalance SME config. Config or path to config file is empty.");

    try {
      getSmeContext().storeConfig(config);
      return RESULT_OK;
    } catch (Exception e) {
      logger.error("Can't store unibalance SME properties", e);
      return error("Can't store unibalance SME config", e);
    }
  }


// Properties ==========================================================================================================


  public String getBalanceResponse() {
    return balanceResponse;
  }

  public void setBalanceResponse(String balanceResponse) {
    this.balanceResponse = balanceResponse;
  }

  public String getBalanceWaitForSmsResponse() {
    return balanceWaitForSmsResponse;
  }

  public void setBalanceWaitForSmsResponse(String balanceWaitForSmsResponse) {
    this.balanceWaitForSmsResponse = balanceWaitForSmsResponse;
  }

  public String getBalanceError() {
    return balanceError;
  }

  public void setBalanceError(String balanceError) {
    this.balanceError = balanceError;
  }

  public String getBalanceCurrencyDefault() {
    return balanceCurrencyDefault;
  }

  public void setBalanceCurrencyDefault(String balanceCurrencyDefault) {
    this.balanceCurrencyDefault = balanceCurrencyDefault;
  }

  public String getBalanceBannerAdd() {
    return balanceBannerAdd;
  }

  public void setBalanceBannerAdd(String balanceBannerAdd) {
    this.balanceBannerAdd = balanceBannerAdd;
  }

  public String getBalanceNegativeResponse() {
    return balanceNegativeResponse;
  }

  public void setBalanceNegativeResponse(String balanceNegativeResponse) {
    this.balanceNegativeResponse = balanceNegativeResponse;
  }

  public String getBalanceWithAccumulatorResponse() {
    return balanceWithAccumulatorResponse;
  }

  public void setBalanceWithAccumulatorResponse(String balanceWithAccumulatorResponse) {
    this.balanceWithAccumulatorResponse = balanceWithAccumulatorResponse;
  }

  public String getBalanceWithAccumulatorNegativeResponse() {
    return balanceWithAccumulatorNegativeResponse;
  }

  public void setBalanceWithAccumulatorNegativeResponse(String balanceWithAccumulatorNegativeResponse) {
    this.balanceWithAccumulatorNegativeResponse = balanceWithAccumulatorNegativeResponse;
  }

  public String getMbStart() {
    return mbStart;
  }

  public void setMbStart(String mbStart) {
    this.mbStart = mbStart;
  }

  public String getMbStop() {
    return mbStop;
  }

  public void setMbStop(String mbStop) {
    this.mbStop = mbStop;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public ArrayList getBalanceCurrencyDescriptions() {
    return balanceCurrencyDescriptions;
  }


// Balance Currency Description ========================================================================================


  public class BalanceCurrencyDescription {
    private final String currencyCode;
    private final String currencyName;

    public BalanceCurrencyDescription(String currencyCode, String currencyName) {
      this.currencyCode = currencyCode != null ? currencyCode : "";
      this.currencyName = currencyName != null ? currencyName : "";
    }

    public String getCurrencyCode() {
      return currencyCode;
    }

    public String getCurrencyName() {
      return currencyName;
    }
  }
}
