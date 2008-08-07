package mobi.eyeline.mcahdb.soap.missedcallservice;
/**
 *  ExtensionMapper class
 */
        
public  class ExtensionMapper{

  public static java.lang.Object getTypeObject(java.lang.String namespaceURI,
                                               java.lang.String typeName,
                                               javax.xml.stream.XMLStreamReader reader) throws java.lang.Exception{

              
    if ("http://tempuri.org/".equals(namespaceURI) && "ArrayOfMissedCall".equals(typeName)) {
      return  ArrayOfMissedCall.Factory.parse(reader);
    }

              
    if ("http://tempuri.org/".equals(namespaceURI) && "MissedCall".equals(typeName)) {
      return  MissedCall.Factory.parse(reader);
    }

              
    throw new org.apache.axis2.databinding.ADBException("Unsupported type " + namespaceURI + " " + typeName);
  }

}
    