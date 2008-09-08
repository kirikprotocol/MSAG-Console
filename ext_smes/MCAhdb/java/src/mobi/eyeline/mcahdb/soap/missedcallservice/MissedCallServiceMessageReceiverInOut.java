package mobi.eyeline.mcahdb.soap.missedcallservice;

import org.apache.axis2.context.MessageContext;
import org.apache.axis2.AxisFault;
import org.apache.axis2.util.JavaUtils;
import org.apache.axis2.description.AxisOperation;
import org.apache.axiom.soap.SOAPEnvelope;

/**
 *  MissedCallServiceMessageReceiverInOut message receiver
 */

public class MissedCallServiceMessageReceiverInOut extends org.apache.axis2.receivers.AbstractInOutMessageReceiver{

  public void invokeBusinessLogic(MessageContext msgContext, MessageContext newMsgContext) throws AxisFault {
    try {
      // get the implementation class for the Web Service
      Object obj = getTheImplementationObject(msgContext);

      MissedCallServiceSkeleton skel = (MissedCallServiceSkeleton)obj;

      //Out Envelop
      SOAPEnvelope envelope;

      //Find the axisOperation that has been set by the Dispatch phase.
      AxisOperation op = msgContext.getOperationContext().getAxisOperation();

      if (op == null)
        throw new org.apache.axis2.AxisFault("Operation is not located, if this is doclit style the SOAP-ACTION should specified via the SOAP Action to use the RawXMLProvider");

      String methodName;

      if ((op.getName() != null) && ((methodName = JavaUtils.xmlNameToJava(op.getName().getLocalPart())) != null)) {

        if("GetMissedCalls".equals(methodName)) {
                
          GetMissedCalls wrappedParam = (GetMissedCalls)fromOM(msgContext.getEnvelope().getBody().getFirstElement(),
                                                               GetMissedCalls.class,
                                                               getEnvelopeNamespaces(msgContext.getEnvelope()));
                                                
          GetMissedCallsResponse getMissedCallsResponse3 = skel.GetMissedCalls(wrappedParam);
                                            
          envelope = toEnvelope(getSOAPFactory(msgContext), getMissedCallsResponse3, false);
                                    
        } else
          throw new java.lang.RuntimeException("method not found");

        newMsgContext.setEnvelope(envelope);
      }
    } catch (java.lang.Exception e) {
      throw AxisFault.makeFault(e);
    }
  }
        
  //
  private static org.apache.axiom.om.OMElement toOM(GetMissedCalls param, boolean optimizeContent) throws org.apache.axis2.AxisFault {
    try{
      return param.getOMElement(GetMissedCalls.MY_QNAME,
        org.apache.axiom.om.OMAbstractFactory.getOMFactory());
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }
  }
        
  private static org.apache.axiom.om.OMElement  toOM(GetMissedCallsResponse param, boolean optimizeContent)
    throws org.apache.axis2.AxisFault {

    try{
      return param.getOMElement(GetMissedCallsResponse.MY_QNAME,
        org.apache.axiom.om.OMAbstractFactory.getOMFactory());
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }
  }
        
  private static org.apache.axiom.soap.SOAPEnvelope toEnvelope(org.apache.axiom.soap.SOAPFactory factory, GetMissedCallsResponse param, boolean optimizeContent)
    throws org.apache.axis2.AxisFault{
    try{
      org.apache.axiom.soap.SOAPEnvelope emptyEnvelope = factory.getDefaultEnvelope();
                           
      emptyEnvelope.getBody().addChild(param.getOMElement(GetMissedCallsResponse.MY_QNAME,factory));

      return emptyEnvelope;
    } catch(org.apache.axis2.databinding.ADBException e){
      throw org.apache.axis2.AxisFault.makeFault(e);
    }
  }
                    
  private static GetMissedCallsResponse wrapGetMissedCalls(){
    return new GetMissedCallsResponse();
  }
                    


  /**
   *  get the default envelope
   */
  private static org.apache.axiom.soap.SOAPEnvelope toEnvelope(org.apache.axiom.soap.SOAPFactory factory){
    return factory.getDefaultEnvelope();
  }


  private static java.lang.Object fromOM(org.apache.axiom.om.OMElement param, java.lang.Class type, java.util.Map extraNamespaces) throws org.apache.axis2.AxisFault{

    try {
      if (GetMissedCalls.class.equals(type))
        return GetMissedCalls.Factory.parse(param.getXMLStreamReaderWithoutCaching());

      if (GetMissedCallsResponse.class.equals(type))
        return GetMissedCallsResponse.Factory.parse(param.getXMLStreamReaderWithoutCaching());

    } catch (java.lang.Exception e) {
      throw org.apache.axis2.AxisFault.makeFault(e);
    }
    return null;
  }

  /**
   *  A utility method that copies the namepaces from the SOAPEnvelope
   */
  private static java.util.Map getEnvelopeNamespaces(org.apache.axiom.soap.SOAPEnvelope env){
    java.util.Map returnMap = new java.util.HashMap(10);
    java.util.Iterator namespaceIterator = env.getAllDeclaredNamespaces();
    while (namespaceIterator.hasNext()) {
      org.apache.axiom.om.OMNamespace ns = (org.apache.axiom.om.OMNamespace) namespaceIterator.next();
      returnMap.put(ns.getPrefix(),ns.getNamespaceURI());
    }
    return returnMap;
  }

  private static org.apache.axis2.AxisFault createAxisFault(java.lang.Exception e) {
    org.apache.axis2.AxisFault f;
    Throwable cause = e.getCause();
    if (cause != null)
      f = new org.apache.axis2.AxisFault(e.getMessage(), cause);
    else
      f = new org.apache.axis2.AxisFault(e.getMessage());

    return f;
  }

}//end of class
    