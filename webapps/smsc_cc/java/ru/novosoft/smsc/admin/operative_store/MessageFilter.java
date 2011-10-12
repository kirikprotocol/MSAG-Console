package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.util.Address;

import java.util.Date;

/**
 * Фильтр сообщений
 * @author Artem Snopkov
 */
public class MessageFilter{

  private Address abonentAddress;
  private Address fromAddress;
  private Address toAddress;
  private String smeId;
  private String srcSmeId;
  private String dstSmeId;
  private String routeId;
  private Long smsId;
  private Date fromDate;
  private Date tillDate;
  private Integer lastResult;
  private int maxRowSize = 1000;

  public MessageFilter() {
  }

  public MessageFilter(MessageFilter o) {
    abonentAddress = o.getAbonentAddress() == null ? null : new Address(o.getAbonentAddress());
    fromAddress = o.getFromAddress() == null ? null : new Address(o.getFromAddress());
    toAddress = o.getToAddress() == null ? null : new Address(o.getToAddress());
    smeId = o.getSmeId();
    srcSmeId = o.getSrcSmeId();
    dstSmeId = o.getDstSmeId();
    routeId = o.getRouteId();
    smsId = o.getSmsId();
    fromDate = o.getFromDate() != null ? new Date(o.getFromDate().getTime()) : null;
    tillDate = o.getTillDate() != null ? new Date(o.getTillDate().getTime()) : null;
    maxRowSize = o.getMaxRowSize();
    lastResult = o.getLastResult();
  }

  public Integer getLastResult() {
    return lastResult;
  }

  public void setLastResult(Integer lastResult) {
    this.lastResult = lastResult;
  }

  public Address getAbonentAddress() {
    return abonentAddress;
  }

  /**
   * Задает фильтр по адресу отправителя или получателя
   * @param abonentAddress адрес отправителя или получателя (может использоваться маска)
   */
  public void setAbonentAddress(Address abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public Address getFromAddress() {
    return fromAddress;
  }

  /**
   * Задает фильтр по адресу отправителя
   * @param fromAddress адрес отправителя (может быть маска)
   */
  public void setFromAddress(Address fromAddress) {
    this.fromAddress = fromAddress;
  }

  public Address getToAddress() {
    return toAddress;
  }

  /**
   * Задает фильтр по адресу получателя
   * @param toAddress адрес получателя (может быть маска)
   */
  public void setToAddress(Address toAddress) {
    this.toAddress = toAddress;
  }

  public String getSmeId() {
    return smeId;
  }

  /**
   * Задает фильтр по srcSmeId или dstSmeId
   * @param smeId srcSmeId или dstSmeId
   */
  public void setSmeId(String smeId) {
    this.smeId = smeId == null || smeId.length() == 0 ? null : smeId;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  /**
   * Задает фильтр по srcSmeId
   * @param srcSmeId идентификатор SME отправителя
   */
  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId == null || srcSmeId.length() == 0 ? null : srcSmeId;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  /**
   *
   * Задает фильтр по dstSmeId
   * @param dstSmeId идентификатор SME-получателя
   */
  public void setDstSmeId(String dstSmeId) {
    this.dstSmeId = dstSmeId == null || dstSmeId.length() == 0 ? null : dstSmeId;
  }

  public String getRouteId() {
    return routeId;
  }

  /**
   * Задает фильтр по идентификатору маршрута
   * @param routeId идентификатор маршрута
   */
  public void setRouteId(String routeId) {
    this.routeId = routeId == null || routeId.length() == 0 ? null : routeId;
  }

  public Long getSmsId() {
    return smsId;
  }

  /**
   * Задает фильтр по идентификатору сообщения
   * @param smsId идентификатор СМС
   */
  public void setSmsId(Long smsId) {
    this.smsId = smsId;
  }

  public Date getFromDate() {
    return fromDate;
  }

  /**
   * Задает фильтр по минимальной дате отправки
   * @param fromDate минимальная дата отправки
   */
  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  /**
   * Задает фильтр по максимальной дате отправки
   * @param tillDate максимальная дата отправки
   */
  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  /**
   * Этот метод может быть переопределен для добавления собственных фильтров
   * @param m сообщение, которое надо проверить
   * @return true, если сообщение удовлетворяет фильтру и false - в противном случае.
   */
  public boolean additionalFilter(Message m) {
    return true;
  }

  /**
   * Максимальное кол-во извлекаемых строк
   * @return максимальное кол-во извлекаемых строк
   */
  public int getMaxRowSize() {
    return maxRowSize;
  }

  /**
   * Устанавливает максимальное кол-во извлекаемых строк
   * @param maxRowSize максимальное кол-во извлекаемых строк
   */
  public void setMaxRowSize(int maxRowSize) {
    this.maxRowSize = maxRowSize;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    MessageFilter that = (MessageFilter) o;

    if (maxRowSize != that.maxRowSize) return false;
    if (abonentAddress != null ? !abonentAddress.equals(that.abonentAddress) : that.abonentAddress != null)
      return false;
    if (dstSmeId != null ? !dstSmeId.equals(that.dstSmeId) : that.dstSmeId != null) return false;
    if (fromAddress != null ? !fromAddress.equals(that.fromAddress) : that.fromAddress != null) return false;
    if (fromDate != null ? !fromDate.equals(that.fromDate) : that.fromDate != null) return false;
    if (lastResult != null ? !lastResult.equals(that.lastResult) : that.lastResult != null) return false;
    if (routeId != null ? !routeId.equals(that.routeId) : that.routeId != null) return false;
    if (smeId != null ? !smeId.equals(that.smeId) : that.smeId != null) return false;
    if (smsId != null ? !smsId.equals(that.smsId) : that.smsId != null) return false;
    if (srcSmeId != null ? !srcSmeId.equals(that.srcSmeId) : that.srcSmeId != null) return false;
    if (tillDate != null ? !tillDate.equals(that.tillDate) : that.tillDate != null) return false;
    if (toAddress != null ? !toAddress.equals(that.toAddress) : that.toAddress != null) return false;

    return true;
  }
}
