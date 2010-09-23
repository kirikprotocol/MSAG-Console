package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.Functions;

import java.util.Date;

/**
 * Фильтр сообщений
 * @author Artem Snopkov
 */
public class MessageFilter {

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
    this.smeId = smeId;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  /**
   * Задает фильтр по srcSmeId
   * @param srcSmeId идентификатор SME отправителя
   */
  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId;
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
    this.dstSmeId = dstSmeId;
  }

  public String getRouteId() {
    return routeId;
  }

  /**
   * Задает фильтр по идентификатору маршрута
   * @param routeId идентификатор маршрута
   */
  public void setRouteId(String routeId) {
    this.routeId = routeId;
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
}
