package ru.novosoft.smsc.infosme.backend.siebel;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.util.Date;

/**
 * author: alkhal
 */
public interface IntegrationDataSource {

  public SmsMail getSmsMail(String clcId) throws IntegrationDataSourceException;
  public ResultSet listSmsMail(String waveId) throws IntegrationDataSourceException;

  public SmsMailParams getSmsMailParams(String waveId) throws IntegrationDataSourceException;
  public ResultSet listSmsMailParams(Date fromUpdate) throws IntegrationDataSourceException;
  public ResultSet listSmsMailParams() throws IntegrationDataSourceException;

  public void setCtrlStatus(String waveId, SmsMailParams.CtrlStatus status) throws IntegrationDataSourceException;
  public SmsMailParams.CtrlStatus getCtrlStatus(String waveId) throws IntegrationDataSourceException;

  public void setSmsMailState(String clcId, SmsMail.MessageState state) throws IntegrationDataSourceException;
  public SmsMail.MessageState getSmsMailState(String clcId) throws IntegrationDataSourceException;

  public void setSmsMailSmscState(String clcId, String code, String value) throws IntegrationDataSourceException;
  public SmsMailSmscState getSmsMailSmscState(String clcId) throws IntegrationDataSourceException;

  public void shutdown();

  public static class SmsMailSmscState{
    private String clcId;
    private String code;
    private String value;

    public SmsMailSmscState(String clcId) {
      this.clcId = clcId;
    }

    public String getClcId() {
      return clcId;
    }

    public String getCode() {
      return code;
    }

    public String getValue() {
      return value;
    }

    public void setClcId(String clcId) {
      this.clcId = clcId;
    }

    public void setCode(String code) {
      this.code = code;
    }

    public void setValue(String value) {
      this.value = value;
    }
  }

}