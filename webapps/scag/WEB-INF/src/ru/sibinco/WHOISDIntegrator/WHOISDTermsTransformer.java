package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.rules.Rule;

import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;
import java.util.*;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 13.03.2006
 * Time: 17:46:39
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDTermsTransformer {
  private final static String HeaderToWHOISDTerms = "<!--whoisd-terms \n <whoisd:terms>";
  private final static String EndingToWHOISDTerms = "</whoisd:terms> \n -->";
  private final static boolean DEBUG = false;

  public static Map buildRules(String service, Reader reader, File xslFolder) throws Exception {
    String content = "";
    String line;
    BufferedReader br = new BufferedReader(reader);
    try {
     while((line=br.readLine()) != null) {
      content = content + line +"\n";
     }
    }
    finally {
      if (br!=null) br.close();
    }
    return buildRules(service,content,xslFolder);
  }

  public static Map buildRules(String service, String content, File xslFolder) throws Exception  {    
    HashMap rules = new HashMap(3);
    Rule newRule;
    String stylesheet = "terms.xsl";
    long id = Long.parseLong(service);
    HashMap transportToXSl = WHOISDRequest.transportToXSl;
    LinkedList termsresult = new LinkedList();
    addBody(termsresult,content, stylesheet, xslFolder);
    termsresult.add(1,HeaderToWHOISDTerms);
    termsresult.add(EndingToWHOISDTerms);
    Vector transports = new Vector(transportToXSl.keySet()) ;

    for (Iterator i = transports.iterator() ;i.hasNext();) {
        LinkedList ruleresult = new LinkedList();
        LinkedList header = new LinkedList();
        LinkedList body = new LinkedList();
        String transport = (String)i.next();
        stylesheet = (String)transportToXSl.get(transport);
        header = Rule.getRuleHeader(id,transport);
        header.removeFirst();
        ruleresult.addAll(header);
        if (addBody(body,content, stylesheet, xslFolder)) {
          System.out.println("WHOISDTermsTransformer: adding Rule "+stylesheet + " : " + transport);
          body.removeFirst();
          body.removeFirst();
          ruleresult.addAll(body);
          newRule = new Rule(new Long(id),"",transport,termsresult);
          newRule.appendBody(ruleresult);
          if (DEBUG) debugRuleBody(newRule);
          rules.put(transport,newRule);
        }
    }
    return rules;
  }


  private static boolean addBody(LinkedList result,String content, String stylesheet, File xslFolder) throws Exception {    
    StringWriter sw = new StringWriter();
    StringReader sr = new StringReader(content);
    BufferedReader br = null;
    try {
    TransformerFactory tFactory = TransformerFactory.newInstance();
    Transformer transformer = tFactory.newTransformer(new StreamSource(new File(xslFolder,stylesheet)) ) ;
    transformer.transform(new StreamSource(sr), new StreamResult(sw));
    br = new BufferedReader(new StringReader(sw.toString()));
    String line;
    while((line=br.readLine()) != null) {
      result.add(line);
    }
    if (DEBUG) debugTransformation(result);
     if (result.size()>2) return true;
    } catch(TransformerException te) {
      throw new WHOISDException("Can't build rules from term, reason: "+te.getMessage());
    }
    finally {
      if (sw!=null) sw.close();
      if (sr!=null) sr.close();
      if (br!=null) br.close();
    }
    return false;
  }

  private static void debugRuleBody(Rule newRule) {
    System.out.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    for (Iterator ii = newRule.getBody().iterator(); ii.hasNext(); )
     System.out.println(ii.next());
    System.out.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  }

  private static void debugTransformation(LinkedList result) {
    System.out.println("=======================================================================");
     for (Iterator ii = result.iterator(); ii.hasNext(); )
     System.out.println(ii.next());
    System.out.println("=======================================================================");
  }
}
