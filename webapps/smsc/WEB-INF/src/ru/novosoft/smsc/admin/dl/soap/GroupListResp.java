package ru.novosoft.smsc.admin.dl.soap;

public class GroupListResp  implements java.io.Serializable {
  private java.lang.String[] groups;

  private int status;

  public GroupListResp() {
  }

  public GroupListResp(
      java.lang.String[] groups,
      int status) {
    this.groups = groups;
    this.status = status;
  }


  /**
   * Gets the groups value for this GroupListResp.
   *
   * @return groups
   */
  public java.lang.String[] getGroups() {
    return groups;
  }


  /**
   * Sets the groups value for this GroupListResp.
   *
   * @param groups
   */
  public void setGroups(java.lang.String[] groups) {
    this.groups = groups;
  }


  /**
   * Gets the status value for this GroupListResp.
   *
   * @return status
   */
  public int getStatus() {
    return status;
  }


  /**
   * Sets the status value for this GroupListResp.
   *
   * @param status
   */
  public void setStatus(int status) {
    this.status = status;
  }

  private java.lang.Object __equalsCalc = null;
  public synchronized boolean equals(java.lang.Object obj) {
    if (!(obj instanceof GroupListResp)) return false;
    GroupListResp other = (GroupListResp) obj;
    if (obj == null) return false;
    if (this == obj) return true;
    if (__equalsCalc != null) {
      return (__equalsCalc == obj);
    }
    __equalsCalc = obj;
    boolean _equals;
    _equals = true &&
        ((this.groups==null && other.getGroups()==null) ||
            (this.groups!=null &&
                java.util.Arrays.equals(this.groups, other.getGroups()))) &&
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
    if (getGroups() != null) {
      for (int i=0;
           i<java.lang.reflect.Array.getLength(getGroups());
           i++) {
        java.lang.Object obj = java.lang.reflect.Array.get(getGroups(), i);
        if (obj != null &&
            !obj.getClass().isArray()) {
          _hashCode += obj.hashCode();
        }
      }
    }
    _hashCode += getStatus();
    __hashCodeCalc = false;
    return _hashCode;
  }

  // Type metadata
  private static org.apache.axis.description.TypeDesc typeDesc =
      new org.apache.axis.description.TypeDesc(GroupListResp.class, true);

  static {
    typeDesc.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "GroupListResp"));
    org.apache.axis.description.ElementDesc elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("groups");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groups"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
    elemField.setNillable(true);
    elemField.setItemQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "item"));
    typeDesc.addFieldDesc(elemField);
    elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("status");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "status"));
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
