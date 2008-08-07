package mobi.eyeline.mcahdb.soap.missedcallservice;
            

/**
 *  MissedCall bean class
 */
        
public  class MissedCall implements org.apache.axis2.databinding.ADBBean {

  private static java.lang.String generatePrefix(java.lang.String namespace) {
    if(namespace.equals("http://tempuri.org/")){
      return "ns1";
    }
    return org.apache.axis2.databinding.utils.BeanUtil.getUniquePrefix();
  }

        

  /**
   * field for PhoneNumber
   */

                        
  protected java.lang.String localPhoneNumber ;
                                
  /*  This tracker boolean wil be used to detect whether the user called the set method
                          *   for this attribute. It will be used to determine whether to include this field
                           *   in the serialized XML
                           */
  protected boolean localPhoneNumberTracker = false ;
                           

  /**
   * Auto generated getter method
   * @return java.lang.String
   */
  public  java.lang.String getPhoneNumber(){
    return localPhoneNumber;
  }

                           
                        
  /**
   * Auto generated setter method
   * @param param PhoneNumber
   */
  public void setPhoneNumber(java.lang.String param){

    localPhoneNumberTracker = param != null;
                                   
    this.localPhoneNumber=param;
  }
                            

  /**
   * field for CallDate
   */

                        
  protected java.lang.String localCallDate ;
                                
  /*  This tracker boolean wil be used to detect whether the user called the set method
                          *   for this attribute. It will be used to determine whether to include this field
                           *   in the serialized XML
                           */
  protected boolean localCallDateTracker = false ;
                           

  /**
   * Auto generated getter method
   * @return java.lang.String
   */
  public  java.lang.String getCallDate(){
    return localCallDate;
  }

                           
                        
  /**
   * Auto generated setter method
   * @param param CallDate
   */
  public void setCallDate(java.lang.String param){

    localCallDateTracker = param != null;
                                   
    this.localCallDate=param;
  }
                            

  /**
   * field for ProcessDate
   */

                        
  protected java.lang.String localProcessDate ;
                                
  /*  This tracker boolean wil be used to detect whether the user called the set method
                          *   for this attribute. It will be used to determine whether to include this field
                           *   in the serialized XML
                           */
  protected boolean localProcessDateTracker = false ;
                           

  /**
   * Auto generated getter method
   * @return java.lang.String
   */
  public  java.lang.String getProcessDate(){
    return localProcessDate;
  }

                           
                        
  /**
   * Auto generated setter method
   * @param param ProcessDate
   */
  public void setProcessDate(java.lang.String param){

    localProcessDateTracker = param != null;
                                   
    this.localProcessDate=param;
  }
                            

  /**
   * field for MissedCalls
   */

                        
  protected int localMissedCalls ;
                                

  /**
   * Auto generated getter method
   * @return int
   */
  public  int getMissedCalls(){
    return localMissedCalls;
  }

                           
                        
  /**
   * Auto generated setter method
   * @param param MissedCalls
   */
  public void setMissedCalls(int param){
                            
    this.localMissedCalls=param;
                                    

  }
                            

  /**
   * field for MessageStatus
   */

                        
  protected long localMessageStatus ;
                                

  /**
   * Auto generated getter method
   * @return long
   */
  public  long getMessageStatus(){
    return localMessageStatus;
  }

                           
                        
  /**
   * Auto generated setter method
   * @param param MessageStatus
   */
  public void setMessageStatus(long param){
                            
    this.localMessageStatus=param;
                                    

  }
                            

  /**
   * isReaderMTOMAware
   * @return true if the reader supports MTOM
   */
  public static boolean isReaderMTOMAware(javax.xml.stream.XMLStreamReader reader) {
    boolean isReaderMTOMAware;
        
    try{
      isReaderMTOMAware = java.lang.Boolean.TRUE.equals(reader.getProperty(org.apache.axiom.om.OMConstants.IS_DATA_HANDLERS_AWARE));
    }catch(java.lang.IllegalArgumentException e){
      isReaderMTOMAware = false;
    }
    return isReaderMTOMAware;
  }
     
     
  /**
   *
   * @param parentQName
   * @param factory
   * @return org.apache.axiom.om.OMElement
   */
  public org.apache.axiom.om.OMElement getOMElement (
    final javax.xml.namespace.QName parentQName,
    final org.apache.axiom.om.OMFactory factory) throws org.apache.axis2.databinding.ADBException{


        
    org.apache.axiom.om.OMDataSource dataSource =
      new org.apache.axis2.databinding.ADBDataSource(this,parentQName){

        public void serialize(org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {
          MissedCall.this.serialize(parentQName,factory,xmlWriter);
        }
      };
    return new org.apache.axiom.om.impl.llom.OMSourcedElementImpl(
      parentQName,factory,dataSource);
            
  }

  public void serialize(final javax.xml.namespace.QName parentQName,
                        final org.apache.axiom.om.OMFactory factory,
                        org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter)
    throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException {
    serialize(parentQName,factory,xmlWriter,false);
  }

  public void serialize(final javax.xml.namespace.QName parentQName,
                        final org.apache.axiom.om.OMFactory factory,
                        org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter,
                        boolean serializeType)
    throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException{
            
                


    java.lang.String prefix = null;
    java.lang.String namespace = null;
                

    prefix = parentQName.getPrefix();
    namespace = parentQName.getNamespaceURI();

    if ((namespace != null) && (namespace.trim().length() > 0)) {
      java.lang.String writerPrefix = xmlWriter.getPrefix(namespace);
      if (writerPrefix != null) {
        xmlWriter.writeStartElement(namespace, parentQName.getLocalPart());
      } else {
        if (prefix == null) {
          prefix = generatePrefix(namespace);
        }

        xmlWriter.writeStartElement(prefix, parentQName.getLocalPart(), namespace);
        xmlWriter.writeNamespace(prefix, namespace);
        xmlWriter.setPrefix(prefix, namespace);
      }
    } else {
      xmlWriter.writeStartElement(parentQName.getLocalPart());
    }
                
    if (serializeType){
               

      java.lang.String namespacePrefix = registerPrefix(xmlWriter,"http://tempuri.org/");
      if ((namespacePrefix != null) && (namespacePrefix.trim().length() > 0)){
        writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","type",
          namespacePrefix+":MissedCall",
          xmlWriter);
      } else {
        writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","type",
          "MissedCall",
          xmlWriter);
      }

               
    }
    if (localPhoneNumberTracker){
      namespace = "http://tempuri.org/";
      if (! namespace.equals("")) {
        prefix = xmlWriter.getPrefix(namespace);

        if (prefix == null) {
          prefix = generatePrefix(namespace);

          xmlWriter.writeStartElement(prefix,"PhoneNumber", namespace);
          xmlWriter.writeNamespace(prefix, namespace);
          xmlWriter.setPrefix(prefix, namespace);

        } else {
          xmlWriter.writeStartElement(namespace,"PhoneNumber");
        }

      } else {
        xmlWriter.writeStartElement("PhoneNumber");
      }
                                

      if (localPhoneNumber==null){
        // write the nil attribute
                                              
        throw new org.apache.axis2.databinding.ADBException("PhoneNumber cannot be null!!");
                                                  
      }else{

                                        
        xmlWriter.writeCharacters(localPhoneNumber);
                                            
      }
                                    
      xmlWriter.writeEndElement();
    } if (localCallDateTracker){
    namespace = "http://tempuri.org/";
    if (! namespace.equals("")) {
      prefix = xmlWriter.getPrefix(namespace);

      if (prefix == null) {
        prefix = generatePrefix(namespace);

        xmlWriter.writeStartElement(prefix,"CallDate", namespace);
        xmlWriter.writeNamespace(prefix, namespace);
        xmlWriter.setPrefix(prefix, namespace);

      } else {
        xmlWriter.writeStartElement(namespace,"CallDate");
      }

    } else {
      xmlWriter.writeStartElement("CallDate");
    }
                                

    if (localCallDate==null){
      // write the nil attribute
                                              
      throw new org.apache.axis2.databinding.ADBException("CallDate cannot be null!!");
                                                  
    }else{

                                        
      xmlWriter.writeCharacters(localCallDate);
                                            
    }
                                    
    xmlWriter.writeEndElement();
  } if (localProcessDateTracker){
    namespace = "http://tempuri.org/";
    if (! namespace.equals("")) {
      prefix = xmlWriter.getPrefix(namespace);

      if (prefix == null) {
        prefix = generatePrefix(namespace);

        xmlWriter.writeStartElement(prefix,"ProcessDate", namespace);
        xmlWriter.writeNamespace(prefix, namespace);
        xmlWriter.setPrefix(prefix, namespace);

      } else {
        xmlWriter.writeStartElement(namespace,"ProcessDate");
      }

    } else {
      xmlWriter.writeStartElement("ProcessDate");
    }
                                

    if (localProcessDate==null){
      // write the nil attribute
                                              
      throw new org.apache.axis2.databinding.ADBException("ProcessDate cannot be null!!");
                                                  
    }else{

                                        
      xmlWriter.writeCharacters(localProcessDate);
                                            
    }
                                    
    xmlWriter.writeEndElement();
  }
    namespace = "http://tempuri.org/";
    if (! namespace.equals("")) {
      prefix = xmlWriter.getPrefix(namespace);

      if (prefix == null) {
        prefix = generatePrefix(namespace);

        xmlWriter.writeStartElement(prefix,"MissedCalls", namespace);
        xmlWriter.writeNamespace(prefix, namespace);
        xmlWriter.setPrefix(prefix, namespace);

      } else {
        xmlWriter.writeStartElement(namespace,"MissedCalls");
      }

    } else {
      xmlWriter.writeStartElement("MissedCalls");
    }
                                
    if (localMissedCalls==java.lang.Integer.MIN_VALUE) {
                                           
      throw new org.apache.axis2.databinding.ADBException("MissedCalls cannot be null!!");
                                                      
    } else {
      xmlWriter.writeCharacters(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localMissedCalls));
    }
                                    
    xmlWriter.writeEndElement();
                             
    namespace = "http://tempuri.org/";
    if (! namespace.equals("")) {
      prefix = xmlWriter.getPrefix(namespace);

      if (prefix == null) {
        prefix = generatePrefix(namespace);

        xmlWriter.writeStartElement(prefix,"MessageStatus", namespace);
        xmlWriter.writeNamespace(prefix, namespace);
        xmlWriter.setPrefix(prefix, namespace);

      } else {
        xmlWriter.writeStartElement(namespace,"MessageStatus");
      }

    } else {
      xmlWriter.writeStartElement("MessageStatus");
    }
                                
    if (localMessageStatus==java.lang.Long.MIN_VALUE) {
                                           
      throw new org.apache.axis2.databinding.ADBException("MessageStatus cannot be null!!");
                                                      
    } else {
      xmlWriter.writeCharacters(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localMessageStatus));
    }
                                    
    xmlWriter.writeEndElement();
                             
    xmlWriter.writeEndElement();
               

  }

  /**
   * Util method to write an attribute with the ns prefix
   */
  private static void writeAttribute(java.lang.String prefix,java.lang.String namespace,java.lang.String attName,
                              java.lang.String attValue,javax.xml.stream.XMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException{
    if (xmlWriter.getPrefix(namespace) == null) {
      xmlWriter.writeNamespace(prefix, namespace);
      xmlWriter.setPrefix(prefix, namespace);

    }

    xmlWriter.writeAttribute(namespace,attName,attValue);

  }

  /**
   * Util method to write an attribute without the ns prefix
   */
  private void writeAttribute(java.lang.String namespace,java.lang.String attName,
                              java.lang.String attValue,javax.xml.stream.XMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException{
    if (namespace.equals(""))
    {
      xmlWriter.writeAttribute(attName,attValue);
    }
    else
    {
      registerPrefix(xmlWriter, namespace);
      xmlWriter.writeAttribute(namespace,attName,attValue);
    }
  }


  /**
   * Util method to write an attribute without the ns prefix
   */
  private void writeQNameAttribute(java.lang.String namespace, java.lang.String attName,
                                   javax.xml.namespace.QName qname, javax.xml.stream.XMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {

    java.lang.String attributeNamespace = qname.getNamespaceURI();
    java.lang.String attributePrefix = xmlWriter.getPrefix(attributeNamespace);
    if (attributePrefix == null) {
      attributePrefix = registerPrefix(xmlWriter, attributeNamespace);
    }
    java.lang.String attributeValue;
    if (attributePrefix.trim().length() > 0) {
      attributeValue = attributePrefix + ":" + qname.getLocalPart();
    } else {
      attributeValue = qname.getLocalPart();
    }

    if (namespace.equals("")) {
      xmlWriter.writeAttribute(attName, attributeValue);
    } else {
      registerPrefix(xmlWriter, namespace);
      xmlWriter.writeAttribute(namespace, attName, attributeValue);
    }
  }
  /**
   *  method to handle Qnames
   */

  private static void writeQName(javax.xml.namespace.QName qname,
                          javax.xml.stream.XMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {
    java.lang.String namespaceURI = qname.getNamespaceURI();
    if (namespaceURI != null) {
      java.lang.String prefix = xmlWriter.getPrefix(namespaceURI);
      if (prefix == null) {
        prefix = generatePrefix(namespaceURI);
        xmlWriter.writeNamespace(prefix, namespaceURI);
        xmlWriter.setPrefix(prefix,namespaceURI);
      }

      if (prefix.trim().length() > 0){
        xmlWriter.writeCharacters(prefix + ":" + org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qname));
      } else {
        // i.e this is the default namespace
        xmlWriter.writeCharacters(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qname));
      }

    } else {
      xmlWriter.writeCharacters(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qname));
    }
  }

  private static void writeQNames(javax.xml.namespace.QName[] qnames,
                           javax.xml.stream.XMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {

    if (qnames != null) {
      // we have to store this data until last moment since it is not possible to write any
      // namespace data after writing the charactor data
      java.lang.StringBuffer stringToWrite = new java.lang.StringBuffer();
      java.lang.String namespaceURI = null;
      java.lang.String prefix = null;

      for (int i = 0; i < qnames.length; i++) {
        if (i > 0) {
          stringToWrite.append(" ");
        }
        namespaceURI = qnames[i].getNamespaceURI();
        if (namespaceURI != null) {
          prefix = xmlWriter.getPrefix(namespaceURI);
          if ((prefix == null) || (prefix.length() == 0)) {
            prefix = generatePrefix(namespaceURI);
            xmlWriter.writeNamespace(prefix, namespaceURI);
            xmlWriter.setPrefix(prefix,namespaceURI);
          }

          if (prefix.trim().length() > 0){
            stringToWrite.append(prefix).append(":").append(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qnames[i]));
          } else {
            stringToWrite.append(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qnames[i]));
          }
        } else {
          stringToWrite.append(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(qnames[i]));
        }
      }
      xmlWriter.writeCharacters(stringToWrite.toString());
    }

  }


  /**
   * Register a namespace prefix
   */
  private static java.lang.String registerPrefix(javax.xml.stream.XMLStreamWriter xmlWriter, java.lang.String namespace) throws javax.xml.stream.XMLStreamException {
    java.lang.String prefix = xmlWriter.getPrefix(namespace);

    if (prefix == null) {
      prefix = generatePrefix(namespace);

      while (xmlWriter.getNamespaceContext().getNamespaceURI(prefix) != null) {
        prefix = org.apache.axis2.databinding.utils.BeanUtil.getUniquePrefix();
      }

      xmlWriter.writeNamespace(prefix, namespace);
      xmlWriter.setPrefix(prefix, namespace);
    }

    return prefix;
  }


  
  /**
   * databinding method to get an XML representation of this object
   *
   */
  public javax.xml.stream.XMLStreamReader getPullParser(javax.xml.namespace.QName qName)
    throws org.apache.axis2.databinding.ADBException{


        
    java.util.ArrayList elementList = new java.util.ArrayList();
    java.util.ArrayList attribList = new java.util.ArrayList();

    if (localPhoneNumberTracker){
      elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
        "PhoneNumber"));
                                 
      if (localPhoneNumber != null){
        elementList.add(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localPhoneNumber));
      } else {
        throw new org.apache.axis2.databinding.ADBException("PhoneNumber cannot be null!!");
      }
    } if (localCallDateTracker){
    elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
      "CallDate"));
                                 
    if (localCallDate != null){
      elementList.add(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localCallDate));
    } else {
      throw new org.apache.axis2.databinding.ADBException("CallDate cannot be null!!");
    }
  } if (localProcessDateTracker){
    elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
      "ProcessDate"));
                                 
    if (localProcessDate != null){
      elementList.add(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localProcessDate));
    } else {
      throw new org.apache.axis2.databinding.ADBException("ProcessDate cannot be null!!");
    }
  }
    elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
      "MissedCalls"));
                                 
    elementList.add(
      org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localMissedCalls));
                            
    elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
      "MessageStatus"));
                                 
    elementList.add(
      org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localMessageStatus));
                            

    return new org.apache.axis2.databinding.utils.reader.ADBXMLStreamReaderImpl(qName, elementList.toArray(), attribList.toArray());
            
            

  }

  

  /**
   *  Factory class that keeps the parse method
   */
  public static class Factory {

    private Factory() {
    }


    /**
     * static method to create the object
     * Precondition:  If this object is an element, the current or next start element starts this object and any intervening reader events are ignorable
     *                If this object is not an element, it is a complex type and the reader is at the event just after the outer start element
     * Postcondition: If this object is an element, the reader is positioned at its end element
     *                If this object is a complex type, the reader is positioned at the end element of its outer element
     */
    public static MissedCall parse(javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{
      MissedCall object = new MissedCall();

//      int event;
//      java.lang.String nillableValue = null;
//      java.lang.String prefix ="";
//      java.lang.String namespaceuri ="";
      try {
                
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();

        if (reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","type") != null) {
          String fullTypeName = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance", "type");

          if (fullTypeName!=null) {
            String nsPrefix = null;
            if (fullTypeName.indexOf(":") > -1)
              nsPrefix = fullTypeName.substring(0,fullTypeName.indexOf(":"));

            nsPrefix = nsPrefix==null ? "" : nsPrefix;

            java.lang.String type = fullTypeName.substring(fullTypeName.indexOf(":")+1);
                    
            if (!"MissedCall".equals(type)){
              //find namespace for the prefix
              java.lang.String nsUri = reader.getNamespaceContext().getNamespaceURI(nsPrefix);
              return (MissedCall) ExtensionMapper.getTypeObject(nsUri,type,reader);
            }
          }
        }

        // Note all attributes that were handled. Used to differ normal attributes
        // from anyAttributes.
//        java.util.Vector handledAttributes = new java.util.Vector();

        reader.next();

        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                                
        if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","PhoneNumber").equals(reader.getName())){
          String content = reader.getElementText();
          object.setPhoneNumber(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));
          reader.next();
                                    
        }  // End of if for expected property start element
                                
                                    
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                                
        if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","CallDate").equals(reader.getName())){
                                
          String content = reader.getElementText();
                                    
          object.setCallDate(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));
                                              
          reader.next();
                                    
        }  // End of if for expected property start element

                                    
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                                
        if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","ProcessDate").equals(reader.getName())){
                                
          String content = reader.getElementText();
                                    
          object.setProcessDate(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));
                                              
          reader.next();
                                    
        }  // End of if for expected property start element
                                    
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                                
        if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","MissedCalls").equals(reader.getName())){
                                
          String content = reader.getElementText();
                                    
          object.setMissedCalls(org.apache.axis2.databinding.utils.ConverterUtil.convertToInt(content));
                                              
          reader.next();
                                    
        }  else
          throw new org.apache.axis2.databinding.ADBException("Unexpected subelement " + reader.getLocalName());
                            
                                    
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                                
        if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","MessageStatus").equals(reader.getName())){
                                
          String content = reader.getElementText();
                                    
          object.setMessageStatus(org.apache.axis2.databinding.utils.ConverterUtil.convertToLong(content));
                                              
          reader.next();
                                    
        } else {

          throw new org.apache.axis2.databinding.ADBException("Unexpected subelement " + reader.getLocalName());
        }
                              
        while (!reader.isStartElement() && !reader.isEndElement())
          reader.next();
                            
        if (reader.isStartElement())
          // A start element we are not expecting indicates a trailing invalid property
          throw new org.apache.axis2.databinding.ADBException("Unexpected subelement " + reader.getLocalName());
                            
      } catch (javax.xml.stream.XMLStreamException e) {
        throw new java.lang.Exception(e);
      }

      return object;
    }

  } //end of factory class

}
           
          