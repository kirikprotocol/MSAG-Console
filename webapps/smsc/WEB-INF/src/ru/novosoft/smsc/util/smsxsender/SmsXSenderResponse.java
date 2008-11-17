/**
 * SmsXSenderResponse.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.novosoft.smsc.util.smsxsender;

public class SmsXSenderResponse  implements java.io.Serializable {
    private java.lang.String id_message;

    private int smpp_code;

    private int status;

    public SmsXSenderResponse() {
    }

    public SmsXSenderResponse(
           java.lang.String id_message,
           int smpp_code,
           int status) {
           this.id_message = id_message;
           this.smpp_code = smpp_code;
           this.status = status;
    }


    /**
     * Gets the id_message value for this SmsXSenderResponse.
     * 
     * @return id_message
     */
    public java.lang.String getId_message() {
        return id_message;
    }


    /**
     * Sets the id_message value for this SmsXSenderResponse.
     * 
     * @param id_message
     */
    public void setId_message(java.lang.String id_message) {
        this.id_message = id_message;
    }


    /**
     * Gets the smpp_code value for this SmsXSenderResponse.
     * 
     * @return smpp_code
     */
    public int getSmpp_code() {
        return smpp_code;
    }


    /**
     * Sets the smpp_code value for this SmsXSenderResponse.
     * 
     * @param smpp_code
     */
    public void setSmpp_code(int smpp_code) {
        this.smpp_code = smpp_code;
    }


    /**
     * Gets the status value for this SmsXSenderResponse.
     * 
     * @return status
     */
    public int getStatus() {
        return status;
    }


    /**
     * Sets the status value for this SmsXSenderResponse.
     * 
     * @param status
     */
    public void setStatus(int status) {
        this.status = status;
    }

    private java.lang.Object __equalsCalc = null;
    public synchronized boolean equals(java.lang.Object obj) {
        if (!(obj instanceof SmsXSenderResponse)) return false;
        SmsXSenderResponse other = (SmsXSenderResponse) obj;
        if (obj == null) return false;
        if (this == obj) return true;
        if (__equalsCalc != null) {
            return (__equalsCalc == obj);
        }
        __equalsCalc = obj;
        boolean _equals;
        _equals = true && 
            ((this.id_message==null && other.getId_message()==null) || 
             (this.id_message!=null &&
              this.id_message.equals(other.getId_message()))) &&
            this.smpp_code == other.getSmpp_code() &&
            this.status == other.getStatus();
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
        if (getId_message() != null) {
            _hashCode += getId_message().hashCode();
        }
        _hashCode += getSmpp_code();
        _hashCode += getStatus();
        __hashCodeCalc = false;
        return _hashCode;
    }

    // Type metadata
    private static org.apache.axis.description.TypeDesc typeDesc =
        new org.apache.axis.description.TypeDesc(SmsXSenderResponse.class, true);

    static {
        typeDesc.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
        org.apache.axis.description.ElementDesc elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("id_message");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "id_message"));
        elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
        elemField.setNillable(true);
        typeDesc.addFieldDesc(elemField);
        elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("smpp_code");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "smpp_code"));
        elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
        elemField.setNillable(false);
        typeDesc.addFieldDesc(elemField);
        elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("status");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "status"));
        elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
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
