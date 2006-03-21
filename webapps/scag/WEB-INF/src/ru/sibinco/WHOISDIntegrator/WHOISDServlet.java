package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.transport.Transport;

import javax.servlet.http.*;
import java.io.*;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 10.03.2006
 * Time: 11:15:17
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDServlet extends HttpServlet {
  private SCAGAppContext appContext;
  private static final String SCAG_ERROR_PREFIX = "Internal MSAG error: ";
  private static final String WHOISD_ERROR_PREFIX = "WHOISD request error: ";

  private static final int CR = (int)'\r';
  private static final int LF = (int)'\n';

  public void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
    appContext = (SCAGAppContext)req.getAttribute("appContext");
    int id = getRequestId(req);
    LinkedList result = new LinkedList();
    try {
    switch (id) {
      case WHOISDRequest.OPERATORS: result = loadXml(appContext.getConfig().getString("operators_file")); SendResult(result, resp); break;
      case WHOISDRequest.SERVICES:  result = loadXml(appContext.getConfig().getString("services_file")); SendResult(result, resp); break;
      case WHOISDRequest.RULE: result = getRule(req);  SendResult(result, resp); break;
      case WHOISDRequest.TARIFF_MATRIX: result = loadXml(appContext.getConfig().getString("tariffs_file"));  SendResult(result, resp); break;
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

  public void doPost(HttpServletRequest req, HttpServletResponse resp) throws IOException{
       appContext = (SCAGAppContext)req.getAttribute("appContext");
       int id = getRequestId(req);
       LinkedList result = new LinkedList();
       try {
        switch (id) {
          case WHOISDRequest.RULE: applyTerm(req,isMultipartFormat(req));SendResult(result, resp); break;
          case WHOISDRequest.TARIFF_MATRIX: saveFile(appContext.getConfig().getString("tariffs_file"),req,isMultipartFormat(req)); SendResult(result, resp); break;
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

   private void applyTerm(HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     RuleManagerWrapper rulemanager = new RuleManagerWrapper(appContext.getRuleManager());
     SAXParserImpl parser = new SAXParserImpl();
     String service = req.getParameter("service");
     if (service == null) throw new WHOISDException("service parameter is missed!");
     try {
       new Long(req.getParameter("service"));
     } catch (NumberFormatException e) {
       throw new WHOISDException("service parameter must be an integer or long value");
     }
     Reader reader = null;
     Map rulesWHOISD = null;
     Rule ruleWHOISD = null;
     if (isMultipartFormat) {
       int[] dataSlice = extractData(req);
       String content = new String();
       for (int i =0 ;i<dataSlice.length;i++)
         content = content +(char)dataSlice[i];
       rulesWHOISD = WHOISDTermsTransformer.buildRules(service, content, rulemanager.getXslFolder());
     } else {
       InputStream in = req.getInputStream();
       reader = new BufferedReader(new InputStreamReader(in));
       rulesWHOISD = WHOISDTermsTransformer.buildRules(service, reader, rulemanager.getXslFolder());
     }
     String[] transports = Transport.transportTitles;
     if (rulesWHOISD.size()>0)
     for (byte i =0 ;i<transports.length;i++) {
       ruleWHOISD = (Rule)rulesWHOISD.get(transports[i]);
       Rule curRule = rulemanager.getRule(new Long(service),transports[i]);
       if (curRule!=null && curRule.isLocked()) throw new WHOISDException("Rule ["+transports[i]+"] is editing in MSAG web admin console right now");
       if (ruleWHOISD == null) {
         if (curRule!=null) rulemanager.removeRule(service,transports[i]);
         continue;
       }
       String ruleSystemId = appContext.getConfig().getString("rules_folder")+"/"+ transports[i] + "/"+ruleWHOISD.getId().toString();
       parser.parse(rulemanager.getRuleContentAsString(ruleWHOISD), ruleSystemId, transports[i]);
       if (ruleWHOISD != null && curRule!=null) {
          LinkedList error = rulemanager.updateRule(ruleWHOISD,service,transports[i]);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(error));
       } else if (ruleWHOISD != null && curRule==null) {
          LinkedList error = rulemanager.AddRule(ruleWHOISD,service);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(error));
       }
     }
   }

   private String composeErrorMessage(LinkedList li) {
     String error=(String)li.get(0);
     return error;
   }

   private void saveFile(String pathToWrite, HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     InputStream in = null;
     BufferedReader br = null;
     FileOutputStream fos = null;
     try {
     fos = new FileOutputStream(pathToWrite);
     if (isMultipartFormat) {
     int[] dataSlice = extractData(req);
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
       if(in!=null) br.close();
       if(in!=null) fos.close();
     }
   }

  private boolean isMultipartFormat(HttpServletRequest req)
  {
             String temptype=req.getContentType();
             if(temptype.indexOf("multipart/form-data")!=-1) return true;
             else return false;
  }

  private int[] extractData(HttpServletRequest request) throws IOException {
      InputStream is = request.getInputStream();
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

  private String getBoundary(HttpServletRequest request) {
    String cType = request.getContentType();
    return cType.substring(cType.indexOf("boundary=")+9);
  }

  private void SendResult(LinkedList result, HttpServletResponse resp) throws IOException {
    if (!resp.containsHeader("status")) {
      resp.setHeader("status","ok");
      if (result.size()>1) resp.setContentType("application/xml");
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
    Rule rule = appContext.getRuleManager().getRule(serviceId,transport);
    if (rule == null) throw new WHOISDException("There is no rule for service with id = " + serviceId + " and transport "+ transport);
    if (rule.isLocked()) throw new WHOISDException("Rule is editing in MSAG web admin console right now");
    LinkedList result = new LinkedList(rule.getBody());
    result.removeFirst();
    return result;
  }

  private LinkedList loadXml(String filepath) throws Exception {
    LinkedList li = new LinkedList();
    File filetoread = new File(filepath);
    InputStream in = null;
    BufferedReader br = null;
    try {
      in = new FileInputStream(filetoread);
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

  private void validateTransportParameter(String transport) throws WHOISDException{
    String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
     if (transport.equals(transports[i])) return;
    }
    throw new WHOISDException("transport parameter '"+transport+"' doesn't correspond to enumeration: "+Arrays.asList(transports));
  }
}