package mobi.eyeline.mcahdb.soap;

        

/*
        *  MissedCallServiceStub java implementation
        */

        
public class MissedCallServiceStub extends org.apache.axis2.client.Stub
{
  protected org.apache.axis2.description.AxisOperation[] _operations;

  //hashmaps to keep the fault mapping
  private java.util.HashMap faultExceptionNameMap = new java.util.HashMap();
  private java.util.HashMap faultExceptionClassNameMap = new java.util.HashMap();
  private java.util.HashMap faultMessageMap = new java.util.HashMap();

  private static int counter = 0;

  private static synchronized String getUniqueSuffix(){
    // reset the counter if it is greater than 99999
    if (counter > 99999){
      counter = 0;
    }
    counter = counter + 1;
    return Long.toString(System.currentTimeMillis()) + "_" + counter;
  }

    
  private void populateAxisService() throws org.apache.axis2.AxisFault {

    //creating the Service with a unique name
    _service = new org.apache.axis2.description.AxisService("MissedCallService" + getUniqueSuffix());
    addAnonymousOperations();

    //creating the operations
    org.apache.axis2.description.AxisOperation __operation;

    _operations = new org.apache.axis2.description.AxisOperation[1];
        
    __operation = new org.apache.axis2.description.OutInAxisOperation();
                

    __operation.setName(new javax.xml.namespace.QName("http://tempuri.org/", "GetMissedCalls"));
    _service.addOperation(__operation);
	    

	    
	    
    _operations[0]=__operation;
            
        
  }

  //populates the faults
  private void populateFaults(){
         


  }

  /**
   *Constructor that takes in a configContext
   */

  public MissedCallServiceStub(org.apache.axis2.context.ConfigurationContext configurationContext,
                               java.lang.String targetEndpoint)
    throws org.apache.axis2.AxisFault {
    this(configurationContext,targetEndpoint,false);
  }


  /**
   * Constructor that takes in a configContext  and useseperate listner
   */
  public MissedCallServiceStub(org.apache.axis2.context.ConfigurationContext configurationContext,
                               java.lang.String targetEndpoint, boolean useSeparateListener)
    throws org.apache.axis2.AxisFault {
    //To populate AxisService
    populateAxisService();
    populateFaults();

    _serviceClient = new org.apache.axis2.client.ServiceClient(configurationContext,_service);
        
	
    configurationContext = _serviceClient.getServiceContext().getConfigurationContext();

    _serviceClient.getOptions().setTo(new org.apache.axis2.addressing.EndpointReference(
      targetEndpoint));
    _serviceClient.getOptions().setUseSeparateListener(useSeparateListener);
        
    //Set the soap version
    _serviceClient.getOptions().setSoapVersionURI(org.apache.axiom.soap.SOAP12Constants.SOAP_ENVELOPE_NAMESPACE_URI);
        
    
  }

  /**
   * Default Constructor
   */
  public MissedCallServiceStub(org.apache.axis2.context.ConfigurationContext configurationContext) throws org.apache.axis2.AxisFault {
        
    this(configurationContext,"http://localhost/MissedCalls.wsdl" );
                
  }

  /**
   * Default Constructor
   */
  public MissedCallServiceStub() throws org.apache.axis2.AxisFault {
        
    this("http://localhost/MissedCalls.wsdl" );
                
  }

  /**
   * Constructor taking the target endpoint
   */
  public MissedCallServiceStub(java.lang.String targetEndpoint) throws org.apache.axis2.AxisFault {
    this(null,targetEndpoint);
  }



        
  /**
   * Auto generated method signature
   *
   * @see mobi.eyeline.mcahdb.soap.MissedCallService#GetMissedCalls
   * @param getMissedCalls
                    
   */

                    

  public  mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse GetMissedCalls(

    mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls getMissedCalls)
                        

    throws java.rmi.RemoteException
                    
  {
    org.apache.axis2.context.MessageContext _messageContext = null;
    try{
      org.apache.axis2.client.OperationClient _operationClient = _serviceClient.createClient(_operations[0].getName());
      _operationClient.getOptions().setAction("http://tempuri.org/GetMissedCalls");
      _operationClient.getOptions().setExceptionToBeThrownOnSOAPFault(true);

              
              
      addPropertyToOperationClient(_operationClient,org.apache.axis2.description.WSDL2Constants.ATTR_WHTTP_QUERY_PARAMETER_SEPARATOR,"&");
              

      // create a message context
      _messageContext = new org.apache.axis2.context.MessageContext();

              

      // create SOAP envelope with that payload
      org.apache.axiom.soap.SOAPEnvelope env = null;
                    
                                                    
      env = toEnvelope(getFactory(_operationClient.getOptions().getSoapVersionURI()),
        getMissedCalls,
        optimizeContent(new javax.xml.namespace.QName("http://tempuri.org/",
          "GetMissedCalls")));
                                                
      //adding SOAP soap_headers
      _serviceClient.addHeadersToEnvelope(env);
      // set the message context with that soap envelope
      _messageContext.setEnvelope(env);

      // add the message contxt to the operation client
      _operationClient.addMessageContext(_messageContext);

      //execute the operation client
      _operationClient.execute(true);

         
      org.apache.axis2.context.MessageContext _returnMessageContext = _operationClient.getMessageContext(
        org.apache.axis2.wsdl.WSDLConstants.MESSAGE_LABEL_IN_VALUE);
      org.apache.axiom.soap.SOAPEnvelope _returnEnv = _returnMessageContext.getEnvelope();
                
                
      java.lang.Object object = fromOM(
        _returnEnv.getBody().getFirstElement() ,
        mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse.class,
        getEnvelopeNamespaces(_returnEnv));

                               
      return (mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse)object;
                                   
    }catch(org.apache.axis2.AxisFault f){

      org.apache.axiom.om.OMElement faultElt = f.getDetail();
      if (faultElt!=null){
        if (faultExceptionNameMap.containsKey(faultElt.getQName())){
          //make the fault by reflection
          try{
            java.lang.String exceptionClassName = (java.lang.String)faultExceptionClassNameMap.get(faultElt.getQName());
            java.lang.Class exceptionClass = java.lang.Class.forName(exceptionClassName);
            java.lang.Exception ex=
              (java.lang.Exception) exceptionClass.newInstance();
            //message class
            java.lang.String messageClassName = (java.lang.String)faultMessageMap.get(faultElt.getQName());
            java.lang.Class messageClass = java.lang.Class.forName(messageClassName);
            java.lang.Object messageObject = fromOM(faultElt,messageClass,null);
            java.lang.reflect.Method m = exceptionClass.getMethod("setFaultMessage",
              new java.lang.Class[]{messageClass});
            m.invoke(ex,new java.lang.Object[]{messageObject});
                        

            throw new java.rmi.RemoteException(ex.getMessage(), ex);
          }catch(java.lang.ClassCastException e){
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          } catch (java.lang.ClassNotFoundException e) {
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          }catch (java.lang.NoSuchMethodException e) {
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          } catch (java.lang.reflect.InvocationTargetException e) {
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          }  catch (java.lang.IllegalAccessException e) {
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          }   catch (java.lang.InstantiationException e) {
            // we cannot intantiate the class - throw the original Axis fault
            throw f;
          }
        }else{
          throw f;
        }
      }else{
        throw f;
      }
    } finally {
      _messageContext.getTransportOut().getSender().cleanup(_messageContext);
    }
  }
            


  /**
   *  A utility method that copies the namepaces from the SOAPEnvelope
   */
  private java.util.Map getEnvelopeNamespaces(org.apache.axiom.soap.SOAPEnvelope env){
    java.util.Map returnMap = new java.util.HashMap();
    java.util.Iterator namespaceIterator = env.getAllDeclaredNamespaces();
    while (namespaceIterator.hasNext()) {
      org.apache.axiom.om.OMNamespace ns = (org.apache.axiom.om.OMNamespace) namespaceIterator.next();
      returnMap.put(ns.getPrefix(),ns.getNamespaceURI());
    }
    return returnMap;
  }

    
    
  private javax.xml.namespace.QName[] opNameArray = null;
  private boolean optimizeContent(javax.xml.namespace.QName opName) {
        

    if (opNameArray == null) {
      return false;
    }
    for (int i = 0; i < opNameArray.length; i++) {
      if (opName.equals(opNameArray[i])) {
        return true;
      }
    }
    return false;
  }
  //http://localhost/MissedCalls.wsdl
  public static class ExtensionMapper{

    public static java.lang.Object getTypeObject(java.lang.String namespaceURI,
                                                 java.lang.String typeName,
                                                 javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{

              
      if (
        "http://tempuri.org/".equals(namespaceURI) &&
          "ArrayOfMissedCall".equals(typeName)){
                   
        return  ArrayOfMissedCall.Factory.parse(reader);
                        

      }

              
      if (
        "http://tempuri.org/".equals(namespaceURI) &&
          "MissedCall".equals(typeName)){
                   
        return  MissedCall.Factory.parse(reader);
                        

      }

              
      throw new org.apache.axis2.databinding.ADBException("Unsupported type " + namespaceURI + " " + typeName);
    }

  }
    
  public static class GetMissedCalls
    implements org.apache.axis2.databinding.ADBBean{
        
    public static final javax.xml.namespace.QName MY_QNAME = new javax.xml.namespace.QName(
      "http://tempuri.org/",
      "GetMissedCalls",
      "ns1");

            

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
                            
      if (param != null){
        //update the setting tracker
        localPhoneNumberTracker = true;
      } else {
        localPhoneNumberTracker = false;
                                              
      }
                                   
      this.localPhoneNumber=param;
                                    

    }
                            

    /**
     * isReaderMTOMAware
     * @return true if the reader supports MTOM
     */
    public static boolean isReaderMTOMAware(javax.xml.stream.XMLStreamReader reader) {
      boolean isReaderMTOMAware = false;
        
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
        new org.apache.axis2.databinding.ADBDataSource(this,MY_QNAME){

          public void serialize(org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {
            GetMissedCalls.this.serialize(MY_QNAME,factory,xmlWriter);
          }
        };
      return new org.apache.axiom.om.impl.llom.OMSourcedElementImpl(
        MY_QNAME,factory,dataSource);
            
    }

    public void serialize(final javax.xml.namespace.QName parentQName,
                          final org.apache.axiom.om.OMFactory factory,
                          org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter)
      throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException{
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
            namespacePrefix+":GetMissedCalls",
            xmlWriter);
        } else {
          writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","type",
            "GetMissedCalls",
            xmlWriter);
        }

               
      }
      if (localPhoneNumberTracker){
        namespace = "http://tempuri.org/";
        if (! namespace.equals("")) {
          prefix = xmlWriter.getPrefix(namespace);

          if (prefix == null) {
            prefix = generatePrefix(namespace);

            xmlWriter.writeStartElement(prefix,"phoneNumber", namespace);
            xmlWriter.writeNamespace(prefix, namespace);
            xmlWriter.setPrefix(prefix, namespace);

          } else {
            xmlWriter.writeStartElement(namespace,"phoneNumber");
          }

        } else {
          xmlWriter.writeStartElement("phoneNumber");
        }
                                

        if (localPhoneNumber==null){
          // write the nil attribute
                                              
          throw new org.apache.axis2.databinding.ADBException("phoneNumber cannot be null!!");
                                                  
        }else{

                                        
          xmlWriter.writeCharacters(localPhoneNumber);
                                            
        }
                                    
        xmlWriter.writeEndElement();
      }
      xmlWriter.writeEndElement();
               

    }

    /**
     * Util method to write an attribute with the ns prefix
     */
    private void writeAttribute(java.lang.String prefix,java.lang.String namespace,java.lang.String attName,
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

    private void writeQName(javax.xml.namespace.QName qname,
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

    private void writeQNames(javax.xml.namespace.QName[] qnames,
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
    private java.lang.String registerPrefix(javax.xml.stream.XMLStreamWriter xmlWriter, java.lang.String namespace) throws javax.xml.stream.XMLStreamException {
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
          "phoneNumber"));
                                 
        if (localPhoneNumber != null){
          elementList.add(org.apache.axis2.databinding.utils.ConverterUtil.convertToString(localPhoneNumber));
        } else {
          throw new org.apache.axis2.databinding.ADBException("phoneNumber cannot be null!!");
        }
      }

      return new org.apache.axis2.databinding.utils.reader.ADBXMLStreamReaderImpl(qName, elementList.toArray(), attribList.toArray());
            
            

    }

  

    /**
     *  Factory class that keeps the parse method
     */
    public static class Factory{

        
        

      /**
       * static method to create the object
       * Precondition:  If this object is an element, the current or next start element starts this object and any intervening reader events are ignorable
       *                If this object is not an element, it is a complex type and the reader is at the event just after the outer start element
       * Postcondition: If this object is an element, the reader is positioned at its end element
       *                If this object is a complex type, the reader is positioned at the end element of its outer element
       */
      public static GetMissedCalls parse(javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{
        GetMissedCalls object =
          new GetMissedCalls();

        int event;
        java.lang.String nillableValue = null;
        java.lang.String prefix ="";
        java.lang.String namespaceuri ="";
        try {
                
          while (!reader.isStartElement() && !reader.isEndElement())
            reader.next();

                
          if (reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","type")!=null){
            java.lang.String fullTypeName = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance",
              "type");
            if (fullTypeName!=null){
              java.lang.String nsPrefix = null;
              if (fullTypeName.indexOf(":") > -1){
                nsPrefix = fullTypeName.substring(0,fullTypeName.indexOf(":"));
              }
              nsPrefix = nsPrefix==null?"":nsPrefix;

              java.lang.String type = fullTypeName.substring(fullTypeName.indexOf(":")+1);
                    
              if (!"GetMissedCalls".equals(type)){
                //find namespace for the prefix
                java.lang.String nsUri = reader.getNamespaceContext().getNamespaceURI(nsPrefix);
                return (GetMissedCalls)ExtensionMapper.getTypeObject(
                  nsUri,type,reader);
              }
                        

            }
                

          }

                

                
          // Note all attributes that were handled. Used to differ normal attributes
          // from anyAttributes.
          java.util.Vector handledAttributes = new java.util.Vector();
                

                 
                    
          reader.next();
                
                                    
          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();
                                
          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","phoneNumber").equals(reader.getName())){
                                
            java.lang.String content = reader.getElementText();
                                    
            object.setPhoneNumber(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));
                                              
            reader.next();
                                    
          }  // End of if for expected property start element
                                
          else {
                                        
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

    }//end of factory class

        

  }
           
          
  public static class ArrayOfMissedCall
    implements org.apache.axis2.databinding.ADBBean{
    /* This type was generated from the piece of schema that had
                name = ArrayOfMissedCall
                Namespace URI = http://tempuri.org/
                Namespace Prefix = ns1
                */
            

    private static java.lang.String generatePrefix(java.lang.String namespace) {
      if(namespace.equals("http://tempuri.org/")){
        return "ns1";
      }
      return org.apache.axis2.databinding.utils.BeanUtil.getUniquePrefix();
    }

        

    /**
     * field for MissedCall
     * This was an Array!
     */

                        
    protected MissedCall[] localMissedCall ;
                                
    /*  This tracker boolean wil be used to detect whether the user called the set method
                          *   for this attribute. It will be used to determine whether to include this field
                           *   in the serialized XML
                           */
    protected boolean localMissedCallTracker = false ;
                           

    /**
     * Auto generated getter method
     * @return MissedCall[]
     */
    public  MissedCall[] getMissedCall(){
      return localMissedCall;
    }

                           
                        


                               
    /**
     * validate the array for MissedCall
     */
    protected void validateMissedCall(MissedCall[] param){
                             
    }


    /**
     * Auto generated setter method
     * @param param MissedCall
     */
    public void setMissedCall(MissedCall[] param){
                              
      validateMissedCall(param);

                               
      if (param != null){
        //update the setting tracker
        localMissedCallTracker = true;
      } else {
        localMissedCallTracker = true;
                                                 
      }
                                      
      this.localMissedCall=param;
    }

                               
                             
    /**
     * Auto generated add method for the array for convenience
     * @param param MissedCall
     */
    public void addMissedCall(MissedCall param){
      if (localMissedCall == null){
        localMissedCall = new MissedCall[]{};
      }

                            
      //update the setting tracker
      localMissedCallTracker = true;
                            

      java.util.List list =
        org.apache.axis2.databinding.utils.ConverterUtil.toList(localMissedCall);
      list.add(param);
      this.localMissedCall =
        (MissedCall[])list.toArray(
          new MissedCall[list.size()]);

    }
                             

    /**
     * isReaderMTOMAware
     * @return true if the reader supports MTOM
     */
    public static boolean isReaderMTOMAware(javax.xml.stream.XMLStreamReader reader) {
      boolean isReaderMTOMAware = false;
        
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
            ArrayOfMissedCall.this.serialize(parentQName,factory,xmlWriter);
          }
        };
      return new org.apache.axiom.om.impl.llom.OMSourcedElementImpl(
        parentQName,factory,dataSource);
            
    }

    public void serialize(final javax.xml.namespace.QName parentQName,
                          final org.apache.axiom.om.OMFactory factory,
                          org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter)
      throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException{
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
            namespacePrefix+":ArrayOfMissedCall",
            xmlWriter);
        } else {
          writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","type",
            "ArrayOfMissedCall",
            xmlWriter);
        }

               
      }
      if (localMissedCallTracker){
        if (localMissedCall!=null){
          for (int i = 0;i < localMissedCall.length;i++){
            if (localMissedCall[i] != null){
              localMissedCall[i].serialize(new javax.xml.namespace.QName("http://tempuri.org/","MissedCall"),
                factory,xmlWriter);
            } else {
                                                   
              // write null attribute
              java.lang.String namespace2 = "http://tempuri.org/";
              if (! namespace2.equals("")) {
                java.lang.String prefix2 = xmlWriter.getPrefix(namespace2);

                if (prefix2 == null) {
                  prefix2 = generatePrefix(namespace2);

                  xmlWriter.writeStartElement(prefix2,"MissedCall", namespace2);
                  xmlWriter.writeNamespace(prefix2, namespace2);
                  xmlWriter.setPrefix(prefix2, namespace2);

                } else {
                  xmlWriter.writeStartElement(namespace2,"MissedCall");
                }

              } else {
                xmlWriter.writeStartElement("MissedCall");
              }

              // write the nil attribute
              writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","nil","1",xmlWriter);
              xmlWriter.writeEndElement();
                                                    
            }

          }
        } else {
                                        
          // write null attribute
          java.lang.String namespace2 = "http://tempuri.org/";
          if (! namespace2.equals("")) {
            java.lang.String prefix2 = xmlWriter.getPrefix(namespace2);

            if (prefix2 == null) {
              prefix2 = generatePrefix(namespace2);

              xmlWriter.writeStartElement(prefix2,"MissedCall", namespace2);
              xmlWriter.writeNamespace(prefix2, namespace2);
              xmlWriter.setPrefix(prefix2, namespace2);

            } else {
              xmlWriter.writeStartElement(namespace2,"MissedCall");
            }

          } else {
            xmlWriter.writeStartElement("MissedCall");
          }

          // write the nil attribute
          writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","nil","1",xmlWriter);
          xmlWriter.writeEndElement();
                                        
        }
      }
      xmlWriter.writeEndElement();
               

    }

    /**
     * Util method to write an attribute with the ns prefix
     */
    private void writeAttribute(java.lang.String prefix,java.lang.String namespace,java.lang.String attName,
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

    private void writeQName(javax.xml.namespace.QName qname,
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

    private void writeQNames(javax.xml.namespace.QName[] qnames,
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
    private java.lang.String registerPrefix(javax.xml.stream.XMLStreamWriter xmlWriter, java.lang.String namespace) throws javax.xml.stream.XMLStreamException {
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

      if (localMissedCallTracker){
        if (localMissedCall!=null) {
          for (int i = 0;i < localMissedCall.length;i++){

            if (localMissedCall[i] != null){
              elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
                "MissedCall"));
              elementList.add(localMissedCall[i]);
            } else {

              elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
                "MissedCall"));
              elementList.add(null);

            }

          }
        } else {

          elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
            "MissedCall"));
          elementList.add(localMissedCall);

        }

      }

      return new org.apache.axis2.databinding.utils.reader.ADBXMLStreamReaderImpl(qName, elementList.toArray(), attribList.toArray());



    }



    /**
     *  Factory class that keeps the parse method
     */
    public static class Factory{




      /**
       * static method to create the object
       * Precondition:  If this object is an element, the current or next start element starts this object and any intervening reader events are ignorable
       *                If this object is not an element, it is a complex type and the reader is at the event just after the outer start element
       * Postcondition: If this object is an element, the reader is positioned at its end element
       *                If this object is a complex type, the reader is positioned at the end element of its outer element
       */
      public static ArrayOfMissedCall parse(javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{
        ArrayOfMissedCall object =
          new ArrayOfMissedCall();

        int event;
        java.lang.String nillableValue = null;
        java.lang.String prefix ="";
        java.lang.String namespaceuri ="";
        try {

          while (!reader.isStartElement() && !reader.isEndElement())
            reader.next();


          if (reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","type")!=null){
            java.lang.String fullTypeName = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance",
              "type");
            if (fullTypeName!=null){
              java.lang.String nsPrefix = null;
              if (fullTypeName.indexOf(":") > -1){
                nsPrefix = fullTypeName.substring(0,fullTypeName.indexOf(":"));
              }
              nsPrefix = nsPrefix==null?"":nsPrefix;

              java.lang.String type = fullTypeName.substring(fullTypeName.indexOf(":")+1);

              if (!"ArrayOfMissedCall".equals(type)){
                //find namespace for the prefix
                java.lang.String nsUri = reader.getNamespaceContext().getNamespaceURI(nsPrefix);
                return (ArrayOfMissedCall)ExtensionMapper.getTypeObject(
                  nsUri,type,reader);
              }


            }


          }




          // Note all attributes that were handled. Used to differ normal attributes
          // from anyAttributes.
          java.util.Vector handledAttributes = new java.util.Vector();




          reader.next();

          java.util.ArrayList list1 = new java.util.ArrayList();


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","MissedCall").equals(reader.getName())){



            // Process the array and step past its final element's end.

            nillableValue = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","nil");
            if ("true".equals(nillableValue) || "1".equals(nillableValue)){
              list1.add(null);
              reader.next();
            } else {
              list1.add(MissedCall.Factory.parse(reader));
            }
            //loop until we find a start element that is not part of this array
            boolean loopDone1 = false;
            while(!loopDone1){
              // We should be at the end element, but make sure
              while (!reader.isEndElement())
                reader.next();
              // Step out of this element
              reader.next();
              // Step to next element event.
              while (!reader.isStartElement() && !reader.isEndElement())
                reader.next();
              if (reader.isEndElement()){
                //two continuous end elements means we are exiting the xml structure
                loopDone1 = true;
              } else {
                if (new javax.xml.namespace.QName("http://tempuri.org/","MissedCall").equals(reader.getName())){

                  nillableValue = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","nil");
                  if ("true".equals(nillableValue) || "1".equals(nillableValue)){
                    list1.add(null);
                    reader.next();
                  } else {
                    list1.add(MissedCall.Factory.parse(reader));
                  }
                }else{
                  loopDone1 = true;
                }
              }
            }
            // call the converter utility  to convert and set the array

            object.setMissedCall((MissedCall[])
              org.apache.axis2.databinding.utils.ConverterUtil.convertToArray(
                MissedCall.class,
                list1));

          }  // End of if for expected property start element

          else {

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

    }//end of factory class



  }


  public static class GetMissedCallsResponse
    implements org.apache.axis2.databinding.ADBBean{

    public static final javax.xml.namespace.QName MY_QNAME = new javax.xml.namespace.QName(
      "http://tempuri.org/",
      "GetMissedCallsResponse",
      "ns1");



    private static java.lang.String generatePrefix(java.lang.String namespace) {
      if(namespace.equals("http://tempuri.org/")){
        return "ns1";
      }
      return org.apache.axis2.databinding.utils.BeanUtil.getUniquePrefix();
    }



    /**
     * field for GetMissedCallsResult
     */


    protected ArrayOfMissedCall localGetMissedCallsResult ;

    /*  This tracker boolean wil be used to detect whether the user called the set method
   *   for this attribute. It will be used to determine whether to include this field
    *   in the serialized XML
    */
    protected boolean localGetMissedCallsResultTracker = false ;


    /**
     * Auto generated getter method
     * @return ArrayOfMissedCall
     */
    public  ArrayOfMissedCall getGetMissedCallsResult(){
      return localGetMissedCallsResult;
    }



    /**
     * Auto generated setter method
     * @param param GetMissedCallsResult
     */
    public void setGetMissedCallsResult(ArrayOfMissedCall param){

      if (param != null){
        //update the setting tracker
        localGetMissedCallsResultTracker = true;
      } else {
        localGetMissedCallsResultTracker = false;

      }

      this.localGetMissedCallsResult=param;


    }


    /**
     * isReaderMTOMAware
     * @return true if the reader supports MTOM
     */
    public static boolean isReaderMTOMAware(javax.xml.stream.XMLStreamReader reader) {
      boolean isReaderMTOMAware = false;

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
        new org.apache.axis2.databinding.ADBDataSource(this,MY_QNAME){

          public void serialize(org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter) throws javax.xml.stream.XMLStreamException {
            GetMissedCallsResponse.this.serialize(MY_QNAME,factory,xmlWriter);
          }
        };
      return new org.apache.axiom.om.impl.llom.OMSourcedElementImpl(
        MY_QNAME,factory,dataSource);

    }

    public void serialize(final javax.xml.namespace.QName parentQName,
                          final org.apache.axiom.om.OMFactory factory,
                          org.apache.axis2.databinding.utils.writer.MTOMAwareXMLStreamWriter xmlWriter)
      throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException{
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
            namespacePrefix+":GetMissedCallsResponse",
            xmlWriter);
        } else {
          writeAttribute("xsi","http://www.w3.org/2001/XMLSchema-instance","type",
            "GetMissedCallsResponse",
            xmlWriter);
        }


      }
      if (localGetMissedCallsResultTracker){
        if (localGetMissedCallsResult==null){
          throw new org.apache.axis2.databinding.ADBException("GetMissedCallsResult cannot be null!!");
        }
        localGetMissedCallsResult.serialize(new javax.xml.namespace.QName("http://tempuri.org/","GetMissedCallsResult"),
          factory,xmlWriter);
      }
      xmlWriter.writeEndElement();


    }

    /**
     * Util method to write an attribute with the ns prefix
     */
    private void writeAttribute(java.lang.String prefix,java.lang.String namespace,java.lang.String attName,
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

    private void writeQName(javax.xml.namespace.QName qname,
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

    private void writeQNames(javax.xml.namespace.QName[] qnames,
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
    private java.lang.String registerPrefix(javax.xml.stream.XMLStreamWriter xmlWriter, java.lang.String namespace) throws javax.xml.stream.XMLStreamException {
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

      if (localGetMissedCallsResultTracker){
        elementList.add(new javax.xml.namespace.QName("http://tempuri.org/",
          "GetMissedCallsResult"));


        if (localGetMissedCallsResult==null){
          throw new org.apache.axis2.databinding.ADBException("GetMissedCallsResult cannot be null!!");
        }
        elementList.add(localGetMissedCallsResult);
      }

      return new org.apache.axis2.databinding.utils.reader.ADBXMLStreamReaderImpl(qName, elementList.toArray(), attribList.toArray());



    }



    /**
     *  Factory class that keeps the parse method
     */
    public static class Factory{




      /**
       * static method to create the object
       * Precondition:  If this object is an element, the current or next start element starts this object and any intervening reader events are ignorable
       *                If this object is not an element, it is a complex type and the reader is at the event just after the outer start element
       * Postcondition: If this object is an element, the reader is positioned at its end element
       *                If this object is a complex type, the reader is positioned at the end element of its outer element
       */
      public static GetMissedCallsResponse parse(javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{
        GetMissedCallsResponse object =
          new GetMissedCallsResponse();

        int event;
        java.lang.String nillableValue = null;
        java.lang.String prefix ="";
        java.lang.String namespaceuri ="";
        try {

          while (!reader.isStartElement() && !reader.isEndElement())
            reader.next();


          if (reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","type")!=null){
            java.lang.String fullTypeName = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance",
              "type");
            if (fullTypeName!=null){
              java.lang.String nsPrefix = null;
              if (fullTypeName.indexOf(":") > -1){
                nsPrefix = fullTypeName.substring(0,fullTypeName.indexOf(":"));
              }
              nsPrefix = nsPrefix==null?"":nsPrefix;

              java.lang.String type = fullTypeName.substring(fullTypeName.indexOf(":")+1);

              if (!"GetMissedCallsResponse".equals(type)){
                //find namespace for the prefix
                java.lang.String nsUri = reader.getNamespaceContext().getNamespaceURI(nsPrefix);
                return (GetMissedCallsResponse)ExtensionMapper.getTypeObject(
                  nsUri,type,reader);
              }


            }


          }




          // Note all attributes that were handled. Used to differ normal attributes
          // from anyAttributes.
          java.util.Vector handledAttributes = new java.util.Vector();




          reader.next();


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","GetMissedCallsResult").equals(reader.getName())){

            object.setGetMissedCallsResult(ArrayOfMissedCall.Factory.parse(reader));

            reader.next();

          }  // End of if for expected property start element

          else {

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

    }//end of factory class



  }


  public static class MissedCall
    implements org.apache.axis2.databinding.ADBBean{
    /* This type was generated from the piece of schema that had
    name = MissedCall
    Namespace URI = http://tempuri.org/
    Namespace Prefix = ns1
    */


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

      if (param != null){
        //update the setting tracker
        localPhoneNumberTracker = true;
      } else {
        localPhoneNumberTracker = false;

      }

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

      if (param != null){
        //update the setting tracker
        localCallDateTracker = true;
      } else {
        localCallDateTracker = false;

      }

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

      if (param != null){
        //update the setting tracker
        localProcessDateTracker = true;
      } else {
        localProcessDateTracker = false;

      }

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
      boolean isReaderMTOMAware = false;

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
      throws javax.xml.stream.XMLStreamException, org.apache.axis2.databinding.ADBException{
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
    private void writeAttribute(java.lang.String prefix,java.lang.String namespace,java.lang.String attName,
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

    private void writeQName(javax.xml.namespace.QName qname,
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

    private void writeQNames(javax.xml.namespace.QName[] qnames,
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
    private java.lang.String registerPrefix(javax.xml.stream.XMLStreamWriter xmlWriter, java.lang.String namespace) throws javax.xml.stream.XMLStreamException {
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
    public static class Factory{




      /**
       * static method to create the object
       * Precondition:  If this object is an element, the current or next start element starts this object and any intervening reader events are ignorable
       *                If this object is not an element, it is a complex type and the reader is at the event just after the outer start element
       * Postcondition: If this object is an element, the reader is positioned at its end element
       *                If this object is a complex type, the reader is positioned at the end element of its outer element
       */
      public static MissedCall parse(javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{
        MissedCall object =
          new MissedCall();

        int event;
        java.lang.String nillableValue = null;
        java.lang.String prefix ="";
        java.lang.String namespaceuri ="";
        try {

          while (!reader.isStartElement() && !reader.isEndElement())
            reader.next();


          if (reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance","type")!=null){
            java.lang.String fullTypeName = reader.getAttributeValue("http://www.w3.org/2001/XMLSchema-instance",
              "type");
            if (fullTypeName!=null){
              java.lang.String nsPrefix = null;
              if (fullTypeName.indexOf(":") > -1){
                nsPrefix = fullTypeName.substring(0,fullTypeName.indexOf(":"));
              }
              nsPrefix = nsPrefix==null?"":nsPrefix;

              java.lang.String type = fullTypeName.substring(fullTypeName.indexOf(":")+1);

              if (!"MissedCall".equals(type)){
                //find namespace for the prefix
                java.lang.String nsUri = reader.getNamespaceContext().getNamespaceURI(nsPrefix);
                return (MissedCall)ExtensionMapper.getTypeObject(
                  nsUri,type,reader);
              }


            }


          }




          // Note all attributes that were handled. Used to differ normal attributes
          // from anyAttributes.
          java.util.Vector handledAttributes = new java.util.Vector();




          reader.next();


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","PhoneNumber").equals(reader.getName())){

            java.lang.String content = reader.getElementText();

            object.setPhoneNumber(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));

            reader.next();

          }  // End of if for expected property start element

          else {

          }


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","CallDate").equals(reader.getName())){

            java.lang.String content = reader.getElementText();

            object.setCallDate(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));

            reader.next();

          }  // End of if for expected property start element

          else {

          }


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","ProcessDate").equals(reader.getName())){

            java.lang.String content = reader.getElementText();

            object.setProcessDate(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToString(content));

            reader.next();

          }  // End of if for expected property start element

          else {

          }


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","MissedCalls").equals(reader.getName())){

            java.lang.String content = reader.getElementText();

            object.setMissedCalls(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToInt(content));

            reader.next();

          }  // End of if for expected property start element

          else{
            // A start element we are not expecting indicates an invalid parameter was passed
            throw new org.apache.axis2.databinding.ADBException("Unexpected subelement " + reader.getLocalName());
          }


          while (!reader.isStartElement() && !reader.isEndElement()) reader.next();

          if (reader.isStartElement() && new javax.xml.namespace.QName("http://tempuri.org/","MessageStatus").equals(reader.getName())){

            java.lang.String content = reader.getElementText();

            object.setMessageStatus(
              org.apache.axis2.databinding.utils.ConverterUtil.convertToLong(content));

            reader.next();

          }  // End of if for expected property start element

          else{
            // A start element we are not expecting indicates an invalid parameter was passed
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

    }//end of factory class



  }


  private  org.apache.axiom.om.OMElement  toOM(mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls param, boolean optimizeContent)
    throws org.apache.axis2.AxisFault {


    try{
      return param.getOMElement(mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls.MY_QNAME,
        org.apache.axiom.om.OMAbstractFactory.getOMFactory());
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }


  }

  private  org.apache.axiom.om.OMElement  toOM(mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse param, boolean optimizeContent)
    throws org.apache.axis2.AxisFault {


    try{
      return param.getOMElement(mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse.MY_QNAME,
        org.apache.axiom.om.OMAbstractFactory.getOMFactory());
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }


  }


  private  org.apache.axiom.soap.SOAPEnvelope toEnvelope(org.apache.axiom.soap.SOAPFactory factory, mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls param, boolean optimizeContent)
    throws org.apache.axis2.AxisFault{


    try{

      org.apache.axiom.soap.SOAPEnvelope emptyEnvelope = factory.getDefaultEnvelope();
      emptyEnvelope.getBody().addChild(param.getOMElement(mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls.MY_QNAME,factory));
      return emptyEnvelope;
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }


  }


  /* methods to provide back word compatibility */




  /**
   *  get the default envelope
   */
  private org.apache.axiom.soap.SOAPEnvelope toEnvelope(org.apache.axiom.soap.SOAPFactory factory){
    return factory.getDefaultEnvelope();
  }


  private  java.lang.Object fromOM(
    org.apache.axiom.om.OMElement param,
    java.lang.Class type,
    java.util.Map extraNamespaces) throws org.apache.axis2.AxisFault{

    try {

      if (mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls.class.equals(type)){

        return mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCalls.Factory.parse(param.getXMLStreamReaderWithoutCaching());


      }

      if (mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse.class.equals(type)){

        return mobi.eyeline.mcahdb.soap.MissedCallServiceStub.GetMissedCallsResponse.Factory.parse(param.getXMLStreamReaderWithoutCaching());


      }

    } catch (java.lang.Exception e) {
      throw org.apache.axis2.AxisFault.makeFault(e);
    }
    return null;
  }




}
   