package ru.novosoft.smsc.admin.operative_store;

import java.util.HashMap;
import java.util.Map;

/**
 * Структура, описывающая дополнительные параметры сообщения
 * @author Artem Snopkov
 */
public class BodyParameters {

  final static int SMSC_BACKUP_SME_TAG = 0;
  final static int SMPP_ESM_CLASS_TAG = 2;
  final static int SMPP_DATA_CODING_TAG = 3;
  final static int SMPP_SHORT_MESSAGE_TAG = 28;
  final static int SMPP_MESSAGE_PAYLOAD_TAG = 29;
  final static int SMPP_CONCAT_INFO_TAG = 40;
  final static int SMSC_MERGE_CONCAT_TAG = 45;
  final static int SMSC_DC_LIST_TAG = 52;
  final static int SMSC_ORIG_PARTS_NUM_TAG = 67;

  private final Map<Integer, Object> parameters = new HashMap<Integer, Object>();

  BodyParameters() {
  }

  void addParameter(int tag, Object value) {
    parameters.put(tag, value);
  }

  public int getOriginalPartsNum() {
    return (Integer)parameters.get(SMSC_ORIG_PARTS_NUM_TAG);
  }

  public byte[] getBackupSme() {
    return (byte[])parameters.get(SMSC_BACKUP_SME_TAG);
  }

  public byte[] getShortMessage() {
    return (byte[])parameters.get(SMPP_SHORT_MESSAGE_TAG);
  }

  public byte[] getMessagePayload() {
    return (byte[])parameters.get(SMPP_MESSAGE_PAYLOAD_TAG);
  }

  public Integer getMergeConcat() {
    return (Integer)parameters.get(SMSC_MERGE_CONCAT_TAG);
  }

  public DataCoding[] getDcList() {
    return (DataCoding[])parameters.get(SMSC_DC_LIST_TAG);
  }

  public byte[] getConcatInfo() {
    return (byte[])parameters.get(SMPP_CONCAT_INFO_TAG);
  }

  public DataCoding getDataCoding() {
    return (DataCoding)parameters.get(SMPP_DATA_CODING_TAG);
  }

  public Integer getEsmClass() {
    return (Integer)parameters.get(SMPP_ESM_CLASS_TAG);
  }

  public Map<Integer, Object> getParams() {
    return new HashMap<Integer, Object>(parameters);
  }
}
