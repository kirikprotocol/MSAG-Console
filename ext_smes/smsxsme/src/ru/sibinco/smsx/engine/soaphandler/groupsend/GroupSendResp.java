/**
 * GroupSendResp.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.groupsend;

public class GroupSendResp  implements java.io.Serializable {
    private ru.sibinco.smsx.engine.soaphandler.groupsend.DeliveryStatus[] deliveryStatuses;

    private java.lang.String msgId;

    private int status;

    public GroupSendResp() {
    }

    public GroupSendResp(
           ru.sibinco.smsx.engine.soaphandler.groupsend.DeliveryStatus[] deliveryStatuses,
           java.lang.String msgId,
           int status) {
           this.deliveryStatuses = deliveryStatuses;
           this.msgId = msgId;
           this.status = status;
    }


    /**
     * Gets the deliveryStatuses value for this GroupSendResp.
     * 
     * @return deliveryStatuses
     */
    public ru.sibinco.smsx.engine.soaphandler.groupsend.DeliveryStatus[] getDeliveryStatuses() {
        return deliveryStatuses;
    }


    /**
     * Sets the deliveryStatuses value for this GroupSendResp.
     * 
     * @param deliveryStatuses
     */
    public void setDeliveryStatuses(ru.sibinco.smsx.engine.soaphandler.groupsend.DeliveryStatus[] deliveryStatuses) {
        this.deliveryStatuses = deliveryStatuses;
    }


    /**
     * Gets the msgId value for this GroupSendResp.
     * 
     * @return msgId
     */
    public java.lang.String getMsgId() {
        return msgId;
    }


    /**
     * Sets the msgId value for this GroupSendResp.
     * 
     * @param msgId
     */
    public void setMsgId(java.lang.String msgId) {
        this.msgId = msgId;
    }


    /**
     * Gets the status value for this GroupSendResp.
     * 
     * @return status
     */
    public int getStatus() {
        return status;
    }


    /**
     * Sets the status value for this GroupSendResp.
     * 
     * @param status
     */
    public void setStatus(int status) {
        this.status = status;
    }

    private java.lang.Object __equalsCalc = null;
    public synchronized boolean equals(java.lang.Object obj) {
        if (!(obj instanceof GroupSendResp)) return false;
        GroupSendResp other = (GroupSendResp) obj;
        if (obj == null) return false;
        if (this == obj) return true;
        if (__equalsCalc != null) {
            return (__equalsCalc == obj);
        }
        __equalsCalc = obj;
        boolean _equals;
        _equals = true && 
            ((this.deliveryStatuses==null && other.getDeliveryStatuses()==null) || 
             (this.deliveryStatuses!=null &&
              java.util.Arrays.equals(this.deliveryStatuses, other.getDeliveryStatuses()))) &&
            ((this.msgId==null && other.getMsgId()==null) || 
             (this.msgId!=null &&
              this.msgId.equals(other.getMsgId()))) &&
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
        if (getDeliveryStatuses() != null) {
            for (int i=0;
                 i<java.lang.reflect.Array.getLength(getDeliveryStatuses());
                 i++) {
                java.lang.Object obj = java.lang.reflect.Array.get(getDeliveryStatuses(), i);
                if (obj != null &&
                    !obj.getClass().isArray()) {
                    _hashCode += obj.hashCode();
                }
            }
        }
        if (getMsgId() != null) {
            _hashCode += getMsgId().hashCode();
        }
        _hashCode += getStatus();
        __hashCodeCalc = false;
        return _hashCode;
    }

    // Type metadata
    private static org.apache.axis.description.TypeDesc typeDesc =
        new org.apache.axis.description.TypeDesc(GroupSendResp.class, true);

    static {
        typeDesc.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "GroupSendResp"));
        org.apache.axis.description.ElementDesc elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("deliveryStatuses");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "deliveryStatuses"));
        elemField.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "DeliveryStatus"));
        elemField.setNillable(true);
        elemField.setItemQName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "item"));
        typeDesc.addFieldDesc(elemField);
        elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("msgId");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "msgId"));
        elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
        elemField.setNillable(true);
        typeDesc.addFieldDesc(elemField);
        elemField = new org.apache.axis.description.ElementDesc();
        elemField.setFieldName("status");
        elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "status"));
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
