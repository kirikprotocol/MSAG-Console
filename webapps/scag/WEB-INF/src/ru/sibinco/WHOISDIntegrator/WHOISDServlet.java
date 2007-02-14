package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.rules.applet.MiscUtilities;
import ru.sibinco.scag.beans.rules.RuleState;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;

import javax.servlet.http.*;
import java.io.*;
import java.util.*;
import java.text.MessageFormat;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 10.03.2006
 * Time: 11:15:17
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDServlet extends HttpServlet {
  protected final Logger logger = Logger.getLogger(this.getClass());

  private SCAGAppContext appContext;
  private HashMap schemaCash = new HashMap();
  private static final String SCAG_ERROR_PREFIX = "Internal MSAG error: ";
  private static final String WHOISD_ERROR_PREFIX = "WHOISD request error: ";

  private static final int CR = (int)'\r';
  private static final int LF = (int)'\n';

  public synchronized void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
    logRequest(req);
    appContext = (SCAGAppContext)req.getAttribute(Constants.APP_CONTEXT);
    int id = getRequestId(req);
    LinkedList result = new LinkedList();
    try {
    switch (id) {
      case WHOISDRequest.OPERATORS: result = loadXml(composePath("gw location.operators_file")); SendResult(result, resp); break;
      case WHOISDRequest.OPERATORS_SCHEMA: result = getSchema(WHOISDRequest.OPERATORS_SCHEMA, composePath("gw location.operators_file")); SendResult(result, resp); break;
      case WHOISDRequest.SERVICES:  result = loadXml(composePath("gw location.services_file")); SendResult(result, resp); break;
      case WHOISDRequest.SERVICES_SCHEMA: result = getSchema(WHOISDRequest.SERVICES_SCHEMA, composePath("gw location.services_file")); SendResult(result, resp); break;
      case WHOISDRequest.RULE: result = getRule(req);  SendResult(result, resp); break;
      case WHOISDRequest.TARIFF_MATRIX: result = loadXml(composePath("gw location.tariffs_file"));  SendResult(result, resp); break;
      case WHOISDRequest.TARIFF_MATRIX_SCHEMA: result = getSchema(WHOISDRequest.TARIFF_MATRIX_SCHEMA,composePath("gw location.tariffs_file"));  SendResult(result, resp); break;
      default:
        resp.setHeader("status","false");
        result.add(WHOISD_ERROR_PREFIX + "Wrong request for get method, available: " + Arrays.asList(WHOISDRequest.WHOISDRequests));
        SendResult(result, resp);
        return;
      }
    }
    catch (Exception e) {
      resp.setHeader("status","false");
      String errorMessage;
      if (e instanceof WHOISDException)
        errorMessage=WHOISD_ERROR_PREFIX + e.getMessage();
      else {
        errorMessage=SCAG_ERROR_PREFIX + e.getMessage();
        e.printStackTrace();
      }
      result.add(errorMessage);
      SendResult(result, resp);
    }
  }

  public synchronized void doPost(HttpServletRequest req, HttpServletResponse resp) throws IOException{
       logRequest(req);
       appContext = (SCAGAppContext)req.getAttribute(Constants.APP_CONTEXT);
       int id = getRequestId(req);
       LinkedList result = new LinkedList();
       try {
        switch (id) {
          case WHOISDRequest.RULE: applyTerm(req,isMultipartFormat(req));SendResult(result, resp); break;
          case WHOISDRequest.TARIFF_MATRIX: appContext.getTMatrixManager().applyTariffMatrix(composePath("gw location.tariffs_file"),req,isMultipartFormat(req), appContext); SendResult(result, resp); break;
          default:
            resp.setHeader("status","false");
            result.add(WHOISD_ERROR_PREFIX+"Wrong request for post method");
            SendResult(result, resp);
            return;
         }
       }
       catch (Exception e) {
         resp.setHeader("status","false");
         String errorMessage;
         if (e instanceof WHOISDException)
           errorMessage=WHOISD_ERROR_PREFIX + e.getMessage();
         else {
           e.printStackTrace();
           errorMessage=SCAG_ERROR_PREFIX + e.getMessage();
         }
         result.add(errorMessage);
         SendResult(result, resp);
       }
  }

   private void logRequest(HttpServletRequest req) {
     logger.debug("Start serving request from WHOISD platform");
     logger.debug("from host - " + req.getRemoteAddr());
     logger.debug("requested url: " + req.getRequestURL());
     logger.debug("method = " + req.getMethod());
   }

   private String composePath(String paramName) throws Exception {
     return appContext.getConfig().getString("gw location.gw_config_folder") + File.separatorChar + appContext.getConfig().getString(paramName);
   }

   private void applyTerm(HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     ByteArrayOutputStream bos = new ByteArrayOutputStream();
     int b;
     InputStream in = req.getInputStream();
     BufferedReader br = new BufferedReader(new InputStreamReader(in));
     while((b=br.read()) != -1) {
         bos.write(b);
     }
     String service = req.getParameter("service");
     if (service == null) throw new WHOISDException("service parameter is missed!");
     try {
       new Long(service);
     } catch (NumberFormatException e) {
       throw new WHOISDException("service parameter must be an integer or long value");
     }
     if (appContext.getServiceProviderManager().getServiceById(new Long(service))==null)
       throw new WHOISDException("MSAG doesn't contain service with id = " + service);     
     RuleManagerWrapper rulemanager = appContext.getRuleManager().getWrapper();
     Map rulesWHOISD = null;
     Rule ruleWHOISD = null;
     LinkedList termAsList = new LinkedList();
     if (isMultipartFormat) {
       int[] dataSlice = extractData(req,new ByteArrayInputStream(bos.toByteArray()));
       String content = new String();
       for (int i =0 ;i<dataSlice.length;i++)
         content = content +(char)dataSlice[i];
         rulesWHOISD = WHOISDTermsTransformer.buildRules(termAsList,service, new BufferedReader(new StringReader(content)), rulemanager.getXslFolder());
     } else {
       rulesWHOISD = WHOISDTermsTransformer.buildRules(termAsList,service, new BufferedReader(new InputStreamReader(new ByteArrayInputStream(bos.toByteArray()))), rulemanager.getXslFolder());
     }
     String[] transports = Transport.transportTitles;
     if (rulesWHOISD.size()>0)
     try {
     for (byte i =0 ;i<transports.length;i++) {
       ruleWHOISD = (Rule)rulesWHOISD.get(transports[i]);
       RuleState curRuleState = rulemanager.getRuleStateAndLock(service,transports[i]);
       if (ruleWHOISD == null) {
         if (curRuleState.getExists()) {
           rulemanager.removeRule(service,transports[i]);
         }
         continue;
       }
       String ruleSystemId = composePath("gw location.rules_folder")+"/"+ transports[i] + "/"+ruleWHOISD.getId().toString();
       try {
        SAXParserImpl.parseRule(rulemanager.getRuleContentAsString(ruleWHOISD), ruleSystemId, transports[i]);
       } catch(WHOISDException e) {
         throw new WHOISDException(composeErrorMessage(ruleWHOISD,termAsList, e.getMessage(), e.getLineNumber()));
       }
       if (ruleWHOISD != null && curRuleState.getExists()) {
          LinkedList error = rulemanager.updateRule(ruleWHOISD,service,transports[i]);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(ruleWHOISD,termAsList, (String)error.get(0) + " Line in term - {0}.", Integer.parseInt((String)error.get(1))) );
       } else if (ruleWHOISD != null && !curRuleState.getExists()) {
          LinkedList error = rulemanager.AddRule(ruleWHOISD,service, transports[i]);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(ruleWHOISD,termAsList, (String)error.get(0) + " Line in term - {0}.", Integer.parseInt((String)error.get(1))) );
       }
     }
     rulemanager.applyTerm(RuleManagerWrapper.TERM_COMMIT);
     } catch (Exception e) {
       rulemanager.applyTerm(RuleManagerWrapper.TERM_ROLLBACK);
       throw e;
     } finally {
       rulemanager.unlockRules();
     }
   }

   private String composeErrorMessage(Rule ruleWHOISD, LinkedList termAsList, String errormessage, int linenumber) {
     int ruleHeaderLength = Rule.mainHeaderLength;
     LinkedList ruleBodyWithoutHeader = ruleWHOISD.getBody();
     for (int ii=0;ii<(ruleHeaderLength+ruleWHOISD.getWhoisdPartOffset());ii++)
       ruleBodyWithoutHeader.removeFirst();   
     int offset = 0;
     int startindex = ruleBodyWithoutHeader.size()-1;
     for (;startindex>0;startindex--){
        ruleBodyWithoutHeader.removeLast();
        offset = Collections.indexOfSubList(termAsList, ruleBodyWithoutHeader);
        if (offset !=-1) break;
     }

     if (WHOISDTermsTransformer.DEBUG) {
     System.out.println("!!!!!!!!!!!!!!! startindex="+startindex);
     System.out.println("+++++++++++++++++++++++++++++++");
     for (Iterator iter=termAsList.iterator();iter.hasNext();){
        System.out.println(iter.next());
     }
     System.out.println("+++++++++++++++++++++++++++++++");

     System.out.println("-------------------------------");
     for (Iterator iter=ruleBodyWithoutHeader.iterator();iter.hasNext();){
        System.out.println(iter.next());
     }
     System.out.println("-------------------------------");
     System.out.println("offset: " + offset);

     System.out.println("e.getLineNumber() - ruleWHOISD.getOffset() - ruleHeaderLength: " + (linenumber - ruleWHOISD.getWhoisdPartOffset() - ruleHeaderLength));
     }

     Object linenumberinterm = null;
     if (offset!=-1)
      linenumberinterm = new Integer(offset + linenumber - ruleWHOISD.getWhoisdPartOffset() - ruleHeaderLength - 1);
     else linenumberinterm = new String("unknown");
     String message = MessageFormat.format(errormessage, new Object[]{linenumberinterm});
     return message;
   }

   public static void saveFile(String pathToWrite, HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     InputStream in = null;
     BufferedReader br = null;
     FileOutputStream fos = null;
     try {
     fos = new FileOutputStream(pathToWrite);
     if (isMultipartFormat) {
     int[] dataSlice = extractData(req, req.getInputStream());
     for(int i=0; i<dataSlice.length; i++)
       fos.write(dataSlice[i]);
     } else {
       in = req.getInputStream();
       br = new BufferedReader(new InputStreamReader(in));
       int line;
       while((line = br.read())!= -1) {
         fos.write(line);
       }
     }
     fos.flush();
     } finally {
       if(in!=null) in.close();
       if(br!=null) br.close();
       if(fos!=null) fos.close();
     }
   }

  private boolean isMultipartFormat(HttpServletRequest req)
  {
       String temptype=req.getContentType();
       if(temptype.indexOf("multipart/form-data")!=-1) return true;
       else return false;
  }

  private static int[] extractData(HttpServletRequest request, InputStream is) throws IOException {
      int[] data = new int[request.getContentLength()];
      int bytes;
      int counter = 0;
      while((bytes=is.read())!=-1) {
        data[counter]=bytes;
        counter++;
      }
      is.close();
      int i;
      int beginSliceIndex = 0;
      int endSliceIndex = data.length - getBoundary(request).length()-9;
      for(i = 0; i < data.length; i++)
      {
          if(data[i] == CR && data[i+1] == LF && data[i+2] == CR && data[i+3] == LF)
          {
            beginSliceIndex = i+4;
            break;
          }
      }

      int[] dataSlice = new int[endSliceIndex-beginSliceIndex+1];
      for(i = beginSliceIndex; i<=endSliceIndex; i++)
      {
        dataSlice[i-beginSliceIndex]=data[i];
      }
      return dataSlice;
  }

  private static String getBoundary(HttpServletRequest request) {
    String cType = request.getContentType();
    return cType.substring(cType.indexOf("boundary=")+9);
  }

  private void SendResult(LinkedList result, HttpServletResponse resp) throws IOException {
    if (!resp.containsHeader("status")) {
      logger.debug("Request is served successfully");
      resp.setHeader("status","ok");
      if (result.size()>1) {
        resp.setContentType("application/xml");
        resp.setCharacterEncoding("UTF-8");
      }
    } else {
      if (((String)result.get(0)).startsWith(SCAG_ERROR_PREFIX)) logger.error("Request is not served, reason: " + result.get(0));
      else logger.debug("Request is not served, reason: " + result.get(0));
    }
    PrintWriter out = null;
    try {
    out = resp.getWriter();
    for (Iterator i = result.iterator(); i.hasNext();)
      out.println(i.next());
    out.flush();
    }
    finally {
     if (out!=null) out.close();
    }
  }

  private int getRequestId(HttpServletRequest req) {
    String[] parsedURI = req.getRequestURI().split("/");
    String requestedFile = parsedURI[parsedURI.length-1];
    return WHOISDRequest.getId(requestedFile);
  }

  private LinkedList getRule(HttpServletRequest req) throws WHOISDException{
    Long serviceId = null;
    if (req.getParameter("service") == null) throw new WHOISDException("service parameter is missed!");
    try {
      serviceId = new Long(req.getParameter("service"));
    } catch (NumberFormatException e) {
      throw new WHOISDException("service parameter must be an integer or long value");
    }
    String transport = req.getParameter("transport");
    if (transport == null) throw new WHOISDException("transport parameter is missed!");
    transport = transport.toUpperCase();
    validateTransportParameter(transport);    
    Rule rule = appContext.getRuleManager().getRule(serviceId.toString(),transport);
    if (rule == null) throw new WHOISDException("There is no rule for service with id = " + serviceId + " and transport "+ transport);
    LinkedList result = new LinkedList(rule.getBody());
    result.removeFirst();
    return result;
  }

  private LinkedList loadXml(String filepath) throws Exception {
    File filetoread = new File(filepath);
    return loadFile(filetoread);
  }

  private LinkedList loadFile(File file) throws Exception {
    LinkedList li = new LinkedList();
    InputStream in = null;
    BufferedReader br = null;
    try {
      in = new FileInputStream(file);
      br = new BufferedReader(new InputStreamReader(in));
      String line;
      while ((line=br.readLine())!=null)
        li.add(line);
    }
    finally {
       if (in!=null) in.close();
       if (br!=null) br.close();
    }
    return li;
  }

  private LinkedList getSchema(int id, String xmlPath) throws Exception{
    RuleManager.Schema schema = (RuleManager.Schema)schemaCash.get(xmlPath);
    File filetoread = new File(xmlPath);
    File schemaFile = new File(filetoread.getParent(), WHOISDRequest.getSchemaName(id));
    if (schema==null || schemaFile.lastModified() > schema.lastmodified) {
       schema = new RuleManager.Schema(schemaFile.lastModified(),loadFile(schemaFile));
       schemaCash.put(xmlPath,schema);
    }
    return schema.schemaContent;
  }

  private void validateTransportParameter(String transport) throws WHOISDException{
    String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
     if (transport.equals(transports[i])) return;
    }
    throw new WHOISDException("transport parameter '"+transport+"' doesn't correspond to enumeration: "+Arrays.asList(transports));
  }
}