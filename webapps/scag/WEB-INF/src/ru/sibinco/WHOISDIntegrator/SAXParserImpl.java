package ru.sibinco.WHOISDIntegrator;

import org.xml.sax.*;
import org.xml.sax.helpers.DefaultHandler;
import java.io.StringReader;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 17.03.2006
 * Time: 17:29:48
 * To change this template use File | Settings | File Templates.
 */

public class SAXParserImpl
{

 //{{{ parse() method
 public void parse(String ruleContent,String path, String transport) throws Exception
 {
  //System.out.println("SAXParserImpl.parse started");

  Handler handler = new Handler();

  XMLReader reader = new org.apache.xerces.parsers.SAXParser();

  reader.setFeature("http://xml.org/sax/features/validation",true);
  reader.setFeature("http://apache.org/xml/features/validation/schema",true);
  reader.setFeature("http://xml.org/sax/features/namespaces",true);
  reader.setFeature("http://apache.org/xml/features/validation/schema-full-checking",true);
  reader.setErrorHandler(handler);
  reader.setEntityResolver(handler);

  InputSource source = new InputSource();
  source.setCharacterStream(new StringReader(ruleContent));
  source.setSystemId(path);

  try
  {
   reader.parse(source);
  }
  catch (SAXException e){
    //e.printStackTrace();
    if (e instanceof SAXParseExceptionEnhanced) {
      SAXParseException spe = (SAXParseException)e.getException();
      //System.out.println("spe.getLineNumber() = " + spe.getLineNumber());
      throw new WHOISDException(e.getMessage()+" while parsing rule ["+transport+"] at line number {0} - "+spe.getMessage(), spe.getLineNumber());
    } else
      throw e;
   }
  //System.out.println("SAXParserImpl.parse parse finished");
 } //}}}


 //{{{ Handler class
 class Handler extends DefaultHandler
 {
  //{{{ resolveEntity() method
  /*public InputSource resolveEntity(String publicId, String systemId)
  throws SAXException
  {
    InputSource source = new InputSource(systemId);
       System.out.println("SAXParserImpl resolveEntity" + " systemId= "+systemId + " publicId = " + publicId);
    return source;
  } */

  //{{{ error() method
  public void error(SAXParseException spe) throws SAXException
  {
    throw new SAXParseExceptionEnhanced("Error",spe);
  } //}}}

  //{{{ warning() method
  public void warning(SAXParseException spe) throws SAXException
  {
    throw new SAXParseExceptionEnhanced("Warning",spe);
  } //}}}

  //{{{ fatalError() method
  public void fatalError(SAXParseException spe) throws SAXException
  {
    throw new SAXParseExceptionEnhanced("Fatal Error",spe);
  } //}}}
 }
 static class SAXParseExceptionEnhanced extends SAXException {
   SAXParseExceptionEnhanced(String prefix, Exception e) {
     super(prefix, e);
   }
 }
}
