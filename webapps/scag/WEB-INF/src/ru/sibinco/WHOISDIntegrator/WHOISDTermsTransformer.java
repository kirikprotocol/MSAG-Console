package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.rules.Rule;

import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;
import java.util.*;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 13.03.2006
 * Time: 17:46:39
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDTermsTransformer {
  protected final static Logger logger = Logger.getLogger(WHOISDTermsTransformer.class);

  private final static String HeaderToWHOISDTerms = "<!--whoisd-terms \n <whoisd:terms>";
  private final static String EndingToWHOISDTerms = "-->";
  public final static boolean DEBUG = false;

    public static final String ENCODING_UTF = "UTF-8";

//  public static Map buildRules_(LinkedList termAsList, String service, BufferedReader br, File xslFolder) throws Exception {
//    String content = "";
//      content = "";
//    String line ="";
//    try {
//     logger.debug("!!!!!!!!!!!Start getting term!!!!!!!!!!");
//        StringBuffer sb = new StringBuffer();
//     while( ( line = br.readLine() ) != null) {
//         sb.append( line ).append( "\n" );
//         termAsList.add(line);
//     }
//     content = sb.toString();
////     logContent(content, "cntbr");
//     logger.debug("\n-----------------------------------------\ncontent=" + content + "\n---------------------------------------\n" +
//             "!!!!!!!!!!!End getting term!!!!!!!!!!");
//    } finally {
//      if (br!=null) br.close();
//    }
//    logger.debug( "WHOISDTermsTransformer.buildRules(4) call buildRules(3)");
//    return buildRules(service,content,xslFolder);
//  }

    public static Map buildRules(LinkedList termAsList,String service, BufferedReader br, File xslFolder) throws Exception {
      String content = "";
      String line ="";
      try {
       logger.debug("!!!!!!!!!!!Start getting term!!!!!!!!!!");
       while((line=br.readLine()) != null) {
        content = content + line + "\n";
         logger.debug(line);
         termAsList.add(line);
       }
       logger.debug("!!!!!!!!!!!End getting term!!!!!!!!!!");
      }
      finally {
        if (br!=null) br.close();
      }
      return buildRules(service,content,xslFolder);
    }

  public static Map buildRules(String service, String content, File xslFolder) throws Exception  {
      logger.debug( "WHOISDTermsTransformer.buildRules(3) start" );
    HashMap rules = new HashMap(3);
    Rule newRule;
    String stylesheet = "terms.xsl";
    long id = Long.parseLong(service);
    HashMap transportToXSl = WHOISDRequest.transportToXSl;
    LinkedList termsresult = new LinkedList();
//      logger.debug( "WHOISDTermsTransformer.buildRules(3) addBody() L65");
    addBody(termsresult,content, stylesheet, xslFolder,false);
    termsresult.remove(1);
    termsresult.add(1,HeaderToWHOISDTerms);
    termsresult.add(EndingToWHOISDTerms);
    Vector transports = new Vector(transportToXSl.keySet()) ;
//      logger.debug( "WHOISDTermsTransformer.buildRules() for");
    for (Iterator i = transports.iterator() ;i.hasNext();) {
        logger.debug( "WHOISDTermsTransformer.buildRules() for into");
        LinkedList ruleresult = new LinkedList();
        LinkedList header = new LinkedList();
        LinkedList body = new LinkedList();
        String transport = (String)i.next();
        stylesheet = (String)transportToXSl.get(transport);
        header = Rule.getRuleHeader(transport);
        header.removeFirst();
        ruleresult.addAll(header);
        if (addBody(body,content, stylesheet, xslFolder,true)) {
          logger.debug("WHOISDTermsTransformer: adding Rule "+stylesheet + " : " + transport);
          //body.removeFirst();
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


  private static boolean addBody(LinkedList result,String content, String stylesheet, File xslFolder, boolean omit_declaration) throws Exception {
      logger.debug( "WHOISDTermsTransformer.addBody() start" + "\ncontent=" + content );
    StringWriter sw = new StringWriter();
    StringReader sr = new StringReader(content);
    BufferedReader br = null;
    try {
        TransformerFactory tFactory = TransformerFactory.newInstance();
        Transformer transformer = tFactory.newTransformer(new StreamSource(new File(xslFolder,stylesheet)) ) ;
        transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, (omit_declaration)?"yes":"no");
//        logContent(content, "cntbr");
        transformer.transform(new StreamSource(sr), new StreamResult(sw));
        br = new BufferedReader(new StringReader(sw.toString()));
        String line;
        while((line=br.readLine()) != null) {
          result.add(line);
        }
//        logger.debug( "WHOISDTermsTransformer.addBody() end try 117");
        if (DEBUG) debugTransformation(result);
//        logger.debug( "WHOISDTermsTransformer.addBody() end try 119");
        // debug code -------------------------
//        for (int i = 0; i < result.size(); i++) {
//            String s = (String) result.get(i);
//            logContent(s, "cntbr");
//        }
        // -------------------------

         if (result.size()>2) return true;
    } catch(TransformerException te) {
        throw new WHOISDException("Can't build rules from term, reason: "+te.getMessage() + " " + te.getLocationAsString());
    }
    finally {
      if (sw!=null) sw.close();
      if (sr!=null) sr.close();
      if (br!=null) br.close();
    }
    return false;
  }

    private static void logContent(String content, String s) {
        try {
            Writer w = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(System.currentTimeMillis() + s + ".log"), "UTF-8"));
            w.write(content);
            w.close();
        } catch (IOException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
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
