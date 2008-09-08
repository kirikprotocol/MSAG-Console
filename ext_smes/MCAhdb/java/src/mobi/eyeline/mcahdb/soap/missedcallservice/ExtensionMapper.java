package mobi.eyeline.mcahdb.soap.missedcallservice;

import javax.xml.stream.XMLStreamReader;

/**
 *  ExtensionMapper class
 */
        
public class ExtensionMapper {

  public static java.lang.Object getTypeObject(String namespaceURI, String typeName, XMLStreamReader reader) throws Exception {

    if ("http://tempuri.org/".equals(namespaceURI) && "ArrayOfMissedCall".equals(typeName))
      return  ArrayOfMissedCall.Factory.parse(reader);
              
    if ("http://tempuri.org/".equals(namespaceURI) && "MissedCall".equals(typeName))
      return  MissedCall.Factory.parse(reader);
              
    throw new org.apache.axis2.databinding.ADBException("Unsupported type " + namespaceURI + ' ' + typeName);
  }

}
    