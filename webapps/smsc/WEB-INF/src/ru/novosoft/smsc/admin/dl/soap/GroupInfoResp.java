package ru.novosoft.smsc.admin.dl.soap;

public class GroupInfoResp  implements java.io.Serializable {
  private java.lang.String[] members;

  private java.lang.String name;

  private java.lang.String owner;

  private int status;

  public GroupInfoResp() {
  }

  public GroupInfoResp(
      java.lang.String[] members,
      java.lang.String name,
      java.lang.String owner,
      int status) {
    this.members = members;
    this.name = name;
    this.owner = owner;
    this.status = status;
  }


  /**
   * Gets the members value for this GroupInfoResp.
   *
   * @return members
   */
  public java.lang.String[] getMembers() {
    return members;
  }


  /**
   * Sets the members value for this GroupInfoResp.
   *
   * @param members
   */
  public void setMembers(java.lang.String[] members) {
    this.members = members;
  }


  /**
   * Gets the name value for this GroupInfoResp.
   *
   * @return name
   */
  public java.lang.String getName() {
    return name;
  }


  /**
   * Sets the name value for this GroupInfoResp.
   *
   * @param name
   */
  public void setName(java.lang.String name) {
    this.name = name;
  }


  /**
   * Gets the owner value for this GroupInfoResp.
   *
   * @return owner
   */
  public java.lang.String getOwner() {
    return owner;
  }


  /**
   * Sets the owner value for this GroupInfoResp.
   *
   * @param owner
   */
  public void setOwner(java.lang.String owner) {
    this.owner = owner;
  }


  /**
   * Gets the status value for this GroupInfoResp.
   *
   * @return status
   */
  public int getStatus() {
    return status;
  }


  /**
   * Sets the status value for this GroupInfoResp.
   *
   * @param status
   */
  public void setStatus(int status) {
    this.status = status;
  }

  private java.lang.Object __equalsCalc = null;
  public synchronized boolean equals(java.lang.Object obj) {
    if (!(obj instanceof GroupInfoResp)) return false;
    GroupInfoResp other = (GroupInfoResp) obj;
    if (obj == null) return false;
    if (this == obj) return true;
    if (__equalsCalc != null) {
      return (__equalsCalc == obj);
    }
    __equalsCalc = obj;
    boolean _equals;
    _equals = true &&
        ((this.members==null && other.getMembers()==null) ||
            (this.members!=null &&
                java.util.Arrays.equals(this.members, other.getMembers()))) &&
        ((this.name==null && other.getName()==null) ||
            (this.name!=null &&
                this.name.equals(other.getName()))) &&
        ((this.owner==null && other.getOwner()==null) ||
            (this.owner!=null &&
                this.owner.equals(other.getOwner()))) &&
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
    if (getMembers() != null) {
      for (int i=0;
           i<java.lang.reflect.Array.getLength(getMembers());
           i++) {
        java.lang.Object obj = java.lang.reflect.Array.get(getMembers(), i);
        if (obj != null &&
            !obj.getClass().isArray()) {
          _hashCode += obj.hashCode();
        }
      }
    }
    if (getName() != null) {
      _hashCode += getName().hashCode();
    }
    if (getOwner() != null) {
      _hashCode += getOwner().hashCode();
    }
    _hashCode += getStatus();
    __hashCodeCalc = false;
    return _hashCode;
  }

  // Type metadata
  private static org.apache.axis.description.TypeDesc typeDesc =
      new org.apache.axis.description.TypeDesc(GroupInfoResp.class, true);

  static {
    typeDesc.setXmlType(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "GroupInfoResp"));
    org.apache.axis.description.ElementDesc elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("members");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "members"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
    elemField.setNillable(true);
    elemField.setItemQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "item"));
    typeDesc.addFieldDesc(elemField);
    elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("name");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "name"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
    elemField.setNillable(true);
    typeDesc.addFieldDesc(elemField);
    elemField = new org.apache.axis.description.ElementDesc();
    elemField.setFieldName("owner");
    elemField.setXmlName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"));
    elemField.setXmlType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"));
    elemField.setNillable(true);
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
