package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

public class CheckSubscriptionResp  implements java.io.Serializable {
  private int status;

  private boolean subscribed;

  public CheckSubscriptionResp() {
  }

  public CheckSubscriptionResp(
    int status,
    boolean subscribed) {
    this.status = status;
    this.subscribed = subscribed;
  }


  /**
   * Gets the status value for this CheckSubscriptionResp.
   *
   * @return status
   */
  public int getStatus() {
    return status;
  }


  /**
   * Sets the status value for this CheckSubscriptionResp.
   *
   * @param status
   */
  public void setStatus(int status) {
    this.status = status;
  }


  /**
   * Gets the subscribed value for this CheckSubscriptionResp.
   *
   * @return subscribed
   */
  public boolean isSubscribed() {
    return subscribed;
  }


  /**
   * Sets the subscribed value for this CheckSubscriptionResp.
   *
   * @param subscribed
   */
  public void setSubscribed(boolean subscribed) {
    this.subscribed = subscribed;
  }

  private java.lang.Object __equalsCalc = null;
  public synchronized boolean equals(java.lang.Object obj) {
    if (!(obj instanceof CheckSubscriptionResp)) return false;
    CheckSubscriptionResp other = (CheckSubscriptionResp) obj;
    if (obj == null) return false;
    if (this == obj) return true;
    if (__equalsCalc != null) {
      return (__equalsCalc == obj);
    }
    __equalsCalc = obj;
    boolean _equals;
    _equals = true &&
      this.status == other.getStatus() &&
      this.subscribed == other.isSubscribed();
    __equalsCalc = null;
    return _equals;
  }

  private boolean __hashCodeCalc = false;
  public synchronized int hashCode() {
    if (__hashCodeCalc) {
      return 0;
    }
    __hashCodeCalc = true;
    int _hashCode = 1;
    _hashCode += getStatus();
    _hashCode += (isSubscribed() ? Boolean.TRUE : Boolean.FALSE).hashCode();
    __hashCodeCalc = false;
    return _hashCode;
  }

  // Type metadata
  private static org.apache.axis.description.TypeDesc typeDesc =
    new org.apache.axis.description.TypeDesc(CheckSubscriptionResp.class, true);

  static {
    typeDesc.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "CheckSubscriptionResp"));
    org.apache.axis.description.ElementDesc elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("status");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "status"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    elemField.setNillable(false);
    typeDesc.addFieldDesc(elemField);
    elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("subscribed");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "subscribed"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"));
    elemField.setNillable(false);
    typeDesc.addFieldDesc(elemField);
  }

  /**
   * Return type metadata object
   */
  public static org.apache.axis.description.TypeDesc getTypeDesc() {
    return typeDesc;
  }

  /**
   * Get Custom Serializer
   */
  public static org.apache.axis.encoding.Serializer getSerializer(
    java.lang.String mechType,
    java.lang.Class _javaType,
    javax.xml.namespace.QName _xmlType) {
    return
      new  org.apache.axis.encoding.ser.BeanSerializer(
        _javaType, _xmlType, typeDesc);
  }

  /**
   * Get Custom Deserializer
   */
  public static org.apache.axis.encoding.Deserializer getDeserializer(
    java.lang.String mechType,
    java.lang.Class _javaType,
    javax.xml.namespace.QName _xmlType) {
    return
      new  org.apache.axis.encoding.ser.BeanDeserializer(
        _javaType, _xmlType, typeDesc);
  }

}
