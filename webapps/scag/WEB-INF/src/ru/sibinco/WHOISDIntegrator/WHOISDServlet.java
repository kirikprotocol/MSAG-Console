package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.rules.applet.MiscUtilities;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.*;
import java.io.*;
import java.util.*;
import java.text.MessageFormat;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 10.03.2006
 * Time: 11:15:17
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDServlet extends HttpServlet {
  private SCAGAppContext appContext;
  private HashMap schemaCash = new HashMap();
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
      case WHOISDRequest.OPERATORS: result = loadXml(WHOISDRequest.OPERATORS, composePath("gw location.operators_file")); SendResult(result, resp); break;
      case WHOISDRequest.OPERATORS_SCHEMA: result = getSchema(WHOISDRequest.OPERATORS_SCHEMA, composePath("gw location.operators_file")); SendResult(result, resp); break;
      case WHOISDRequest.SERVICES:  result = loadXml(WHOISDRequest.SERVICES, composePath("gw location.services_file")); SendResult(result, resp); break;
      case WHOISDRequest.SERVICES_SCHEMA: result = getSchema(WHOISDRequest.SERVICES_SCHEMA, composePath("gw location.services_file")); SendResult(result, resp); break;
      case WHOISDRequest.RULE: result = getRule(req);  SendResult(result, resp); break;
      case WHOISDRequest.TARIFF_MATRIX: result = loadXml(WHOISDRequest.TARIFF_MATRIX, composePath("gw location.tariffs_file"));  SendResult(result, resp); break;
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

  public void doPost(HttpServletRequest req, HttpServletResponse resp) throws IOException{
       appContext = (SCAGAppContext)req.getAttribute("appContext");
       int id = getRequestId(req);
       LinkedList result = new LinkedList();
       try {
        switch (id) {
          case WHOISDRequest.RULE: applyTerm(req,isMultipartFormat(req));SendResult(result, resp); break;
          case WHOISDRequest.TARIFF_MATRIX: applyTariffMatrix(composePath("gw location.tariffs_file"),req,isMultipartFormat(req)); SendResult(result, resp); break;
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

   private final String composePath(String paramName) throws Exception {
     return appContext.getConfig().getString("gw location.gw_config_folder") + File.separatorChar + appContext.getConfig().getString(paramName);
   }

   private void applyTerm(HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     String service = req.getParameter("service");
     if (service == null) throw new WHOISDException("service parameter is missed!");
     try {
       new Long(req.getParameter("service"));
     } catch (NumberFormatException e) {
       throw new WHOISDException("service parameter must be an integer or long value");
     }
     RuleManagerWrapper rulemanager = new RuleManagerWrapper(appContext.getRuleManager());
     SAXParserImpl parser = new SAXParserImpl();
     Reader reader = null;
     Map rulesWHOISD = null;
     Rule ruleWHOISD = null;
     LinkedList WHOISDpart = new LinkedList();
     if (isMultipartFormat) {
       int[] dataSlice = extractData(req);
       String content = new String();
       for (int i =0 ;i<dataSlice.length;i++)
         content = content +(char)dataSlice[i];
       rulesWHOISD = WHOISDTermsTransformer.buildRules(WHOISDpart,service, content, rulemanager.getXslFolder());
     } else {
       InputStream in = req.getInputStream();
       reader = new BufferedReader(new InputStreamReader(in));
       rulesWHOISD = WHOISDTermsTransformer.buildRules(WHOISDpart,service, reader, rulemanager.getXslFolder());
     }
     String[] transports = Transport.transportTitles;
     int termoffset = 0;
     if (rulesWHOISD.size()>0)
     for (byte i =0 ;i<transports.length;i++) {
       ruleWHOISD = (Rule)rulesWHOISD.get(transports[i]);
       Rule curRule = rulemanager.getRule(new Long(service),transports[i]);
       if (curRule!=null && curRule.isLocked()) throw new WHOISDException("Rule ["+transports[i]+"] is editing in MSAG web admin console right now");
       if (ruleWHOISD == null) {
         if (curRule!=null) rulemanager.removeRule(service,transports[i]);
         continue;
       }
       String ruleSystemId = composePath("gw location.rules_folder")+"/"+ transports[i] + "/"+ruleWHOISD.getId().toString();
       try {
        parser.parseRule(rulemanager.getRuleContentAsString(ruleWHOISD), ruleSystemId, transports[i]);
       } catch(WHOISDException e) {
         int linenumberinterm = termoffset + e.getLineNumber();
         //System.out.println("linenumberinterm: " + linenumberinterm);
         String message = MessageFormat.format(e.getMessage(), new Object[]{new Integer(linenumberinterm)});
         throw new WHOISDException(message);
       }
       if (ruleWHOISD != null && curRule!=null) {
          LinkedList error = rulemanager.updateRule(ruleWHOISD,service,transports[i]);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(error));
       } else if (ruleWHOISD != null && curRule==null) {
          LinkedList error = rulemanager.AddRule(ruleWHOISD,service);
          if (error != null && error.size()>0)
            throw new WHOISDException(composeErrorMessage(error));
       }
       termoffset = termoffset+ruleWHOISD.getBody().size() - WHOISDpart.size();
     }
   }

   private final String composeErrorMessage(LinkedList li) {
     String error=(String)li.get(0);
     return error;
   }

   private void applyTariffMatrix(String pathToWrite, HttpServletRequest req, boolean isMultipartFormat) throws Exception {
     //backup current file "tariffs.xml" -> "tariffs.xml~"
     File tariffMatrix = new File(pathToWrite);
     File tariffMatrixbackup = new File(pathToWrite+'~');
     boolean backupResult = false;
     // do we have file to backup?
     if (tariffMatrix.exists()) {
      backupResult = MiscUtilities.moveFile(tariffMatrix,tariffMatrixbackup, false);
      if (!backupResult) throw new Exception("Can't save backup of tariffs.xml");
     }
     saveFile(pathToWrite, req, isMultipartFormat);
     SAXParserImpl parser = new SAXParserImpl();
     try {
     parser.parseTariffMatrix(pathToWrite);
     //there are no exceptions -> just delete backup file
     tariffMatrixbackup.delete();
     } catch (Exception e) {
       //excpiton occurs while parsing -> restore backup file(i.e. "tariffs.xml~" ->  "tariffs.xml") and delete backup file
       if (tariffMatrixbackup.exists()) {
         if (!MiscUtilities.moveFile(tariffMatrixbackup,tariffMatrix, true)) throw new Exception("Can't restore tariffs.xml from tariffs.xml~");
       } else
         if (!tariffMatrix.delete()) throw new Exception("TariffMatrix contains errors but can not be deleted");
       throw e;
     }
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
       if(br!=null) br.close();
       if(fos!=null) fos.close();
     }
   }

  private final boolean isMultipartFormat(HttpServletRequest req)
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

  private final String getBoundary(HttpServletRequest request) {
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

  private final int getRequestId(HttpServletRequest req) {
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

  private LinkedList loadXml(int commandId, String filepath) throws Exception {
    String scagCommand = WHOISDRequest.getScagCommand(commandId);
    Method command = appContext.getScag().getClass().getMethod(scagCommand, new Class[0]);
    try {
     command.invoke(appContext.getScag(), new Object[0]);
    } catch(InvocationTargetException e) {
      if (Proxy.STATUS_CONNECTED == appContext.getScag().getStatus())
        throw (Exception)e.getTargetException();
    }
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
    LinkedList schema = (LinkedList)schemaCash.get(xmlPath);
    if (schema!=null)  return schema;
    File filetoread = new File(xmlPath);
    File schemaFile = new File(filetoread.getParent(), WHOISDRequest.getSchemaName(id));
    schema = loadFile(schemaFile);
    schemaCash.put(xmlPath,schema);
    return schema;
  }

  private final void validateTransportParameter(String transport) throws WHOISDException{
    String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
     if (transport.equals(transports[i])) return;
    }
    throw new WHOISDException("transport parameter '"+transport+"' doesn't correspond to enumeration: "+Arrays.asList(transports));
  }
}