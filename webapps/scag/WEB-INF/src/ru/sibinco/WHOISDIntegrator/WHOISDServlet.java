package ru.sibinco.WHOISDIntegrator;

import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.rules.RuleState;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletOutputStream;
import java.io.*;
import java.text.MessageFormat;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 10.03.2006
 * Time: 11:15:17
 * To change this template use File | Settings | File Templates.
 */
public class WHOISDServlet extends HttpServlet
{
  protected final Logger logger = Logger.getLogger(this.getClass());

//  private SCAGAppContext appContext;
  private HashMap schemaCash = new HashMap();
  private static final String SCAG_ERROR_PREFIX = "Internal MSAG error: ";
  private static final String WHOISD_ERROR_PREFIX = "WHOISD request error: ";
  public static final String GW_LOCATION_OPERATORS_FILE = "gw location.operators_file";
  public static final String GW_LOCATION_SERVICES_FILE =  "gw location.services_file";
  public static final String GW_LOCATION_TARIFFS_FILE =   "gw location.tariffs_file";
  public static final String GW_LOCATION_CONFIG_FOLDER =  "gw location.gw_config_folder";
  public static final String GW_LOCATION_RULES_FOLDER =   "gw location.rules_folder";
  public static final String CONTENT_TYPE_XML = "application/xhtml+xml";
  public static final String CONTENT_TYPE_TXT = "text/plain";

  public static final Object ruleLock =    new Object();
  public static final Object tariffsLock = new Object();

  private static final int CR = (int)'\r';
  private static final int LF = (int)'\n';

  private class WHOISDResult
  {
    public byte result[] = null;
    public String error = null;
  }

  private WHOISDResult addError(String error)
  {
    WHOISDResult result = new WHOISDResult();
    result.error = error;
    return result;
  }

  public void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException
  {
    logger.debug( "WHOISDServlet.doGet() start" );
    logRequest(req);
    SCAGAppContext appContext = (SCAGAppContext)req.getAttribute(Constants.APP_CONTEXT);
    int id = getRequestId(req);
    WHOISDResult result = null;
    String contentType = CONTENT_TYPE_TXT;
    try {
    switch (id) {
      case WHOISDRequest.OPERATORS:
        result = loadXml(composePath(GW_LOCATION_OPERATORS_FILE, appContext));
        contentType = CONTENT_TYPE_XML;
        break;
      case WHOISDRequest.OPERATORS_SCHEMA:
        result = getSchema(id, composePath(GW_LOCATION_OPERATORS_FILE, appContext));
        contentType = CONTENT_TYPE_TXT;
        break;
      case WHOISDRequest.SERVICES:
        result = loadXml(composePath(GW_LOCATION_SERVICES_FILE, appContext));
        contentType = CONTENT_TYPE_XML;
        break;
      case WHOISDRequest.SERVICES_SCHEMA:
        result = getSchema(id, composePath(GW_LOCATION_SERVICES_FILE, appContext));
        contentType = CONTENT_TYPE_TXT;
        break;
      case WHOISDRequest.RULE:
        synchronized(ruleLock) {
            result = getRule(req, appContext);
        }
        contentType = CONTENT_TYPE_TXT;
        break;
      case WHOISDRequest.TARIFF_MATRIX:
        synchronized(tariffsLock) {
            result = loadXml(composePath(GW_LOCATION_TARIFFS_FILE, appContext));
        }
        contentType = CONTENT_TYPE_XML;
        break;
      case WHOISDRequest.TARIFF_MATRIX_SCHEMA:
        result = getSchema(id, composePath(GW_LOCATION_TARIFFS_FILE, appContext));
        contentType = CONTENT_TYPE_TXT;
        break;
      default:
        resp.setHeader("status","false");
        result = addError(WHOISD_ERROR_PREFIX + "Wrong request for get method, available: "
                          + Arrays.asList(WHOISDRequest.WHOISDRequests));
        contentType = CONTENT_TYPE_TXT;
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
      result = addError(errorMessage);
    }

    SendResult(result, resp, contentType);
  }

  public synchronized void doPost(HttpServletRequest req, HttpServletResponse resp) throws IOException
  {
      logger.debug( "WHOISDServlet.doPost() start" );
      req.setCharacterEncoding( "UTF-8" );
      logRequest(req);
      SCAGAppContext appContext = (SCAGAppContext)req.getAttribute(Constants.APP_CONTEXT);
      int id = getRequestId(req);
      WHOISDResult result = new WHOISDResult();

      try {
        switch (id) {
          case WHOISDRequest.RULE:
              synchronized(ruleLock){
                  logger.debug( "WHOISDServlet.doPost() applyterm() isMultipartFormat(req)=" + isMultipartFormat(req)  );
                  applyTerm(req, isMultipartFormat(req), appContext);
              }
              break;
          case WHOISDRequest.TARIFF_MATRIX:
              synchronized(tariffsLock){
                logger.debug( "WHOISDServlet.doPost() tariffsLock" );
                appContext.getTMatrixManager().applyTariffMatrix(
                            composePath(GW_LOCATION_TARIFFS_FILE,
                            appContext), req, isMultipartFormat(req),
                            appContext );
              }
            break;
          default:
            resp.setHeader("status","false");
            result = addError(WHOISD_ERROR_PREFIX + "Wrong request for post method");
         }
      }
      catch (Exception e) {
         logger.debug( "WHOISDServlet.doPost() Exception" );
         resp.setHeader("status","false");
         String errorMessage;
         if (e instanceof WHOISDException)
           errorMessage=WHOISD_ERROR_PREFIX + e.getMessage();
         else {
           e.printStackTrace();
           errorMessage=SCAG_ERROR_PREFIX + e.getMessage();
         }
         result = addError(errorMessage);
      }
      SendResult(result, resp, "");
  }

   private void logRequest(HttpServletRequest req)
   {
     logger.debug("Start serving request from WHOISD platform");
     logger.debug("from host - " + req.getRemoteAddr());
     logger.debug("requested url: " + req.getRequestURL());
     logger.debug("method = " + req.getMethod());
   }

   private String composePath(String paramName, SCAGAppContext appContext) throws Exception {
     return appContext.getConfig().getString(GW_LOCATION_CONFIG_FOLDER) +
            File.separatorChar + appContext.getConfig().getString(paramName);
   }

    public static String ENCO_UTF_8="UTF-8";

    private void applyTerm(HttpServletRequest req, boolean isMultipartFormat, SCAGAppContext appContext) throws Exception
    {
      logger.debug( "WHOISDServlet.applyTerm() start\n\t encoding='" + req.getCharacterEncoding() );
      String encoding = ( req.getCharacterEncoding()!=null? req.getCharacterEncoding(): ENCO_UTF_8 );

      InputStream reqIS = req.getInputStream();
      ByteArrayOutputStream baos = new ByteArrayOutputStream();

//      BufferedReader br = new BufferedReader(new InputStreamReader(in));
      BufferedReader br = null;
      BufferedWriter bw = null;
      try {
          br = new BufferedReader( new InputStreamReader(reqIS, encoding) );
          bw = new BufferedWriter( new OutputStreamWriter(baos, encoding) );
      } catch (UnsupportedEncodingException e) {
          logger.debug( "WHOISDServlet.applyTerm() UnsupportedEncodingException" );
          encoding = ENCO_UTF_8;
          e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      }

      int b;
      try {
          while((b=br.read()) != -1) {
//          bos.write(b);
              bw.write(b);
          }
      } finally {
          if( bw!= null ) bw.close();
          if( br!= null ) br.close();
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
      RuleManagerWrapper ruleManagerWrap = appContext.getRuleManager().getWrapper();
      Map rulesWHOISD = null;
      Rule ruleWHOISD = null;
      LinkedList termAsList = new LinkedList();
      if( isMultipartFormat ) {
        logger.debug( "WHOISDServlet.applyTerm() isMultipartFormat" );
        int[] dataSlice = extractData(req,new ByteArrayInputStream(baos.toByteArray()));
        String content = new String();
        for (int i=0 ;i<dataSlice.length;i++)
          content = content +(char)dataSlice[i];
        rulesWHOISD = WHOISDTermsTransformer.buildRules(termAsList,service,
                new BufferedReader(new StringReader(content)), ruleManagerWrap.getXslFolder());
        logger.debug( "WHOISDServlet.applyTerm() isMultipartFormat rylesWHOISD='" + rulesWHOISD + "'" );
      } else {
        logger.debug( "WHOISDServlet.applyTerm() (!isMultipartFormat)" );
        rulesWHOISD = WHOISDTermsTransformer.buildRules(termAsList,service,
                new BufferedReader( new InputStreamReader( new ByteArrayInputStream(baos.toByteArray() ), encoding) ),
                ruleManagerWrap.getXslFolder());
        logger.debug( "WHOISDServlet.applyTerm() rylesWHOISD='" + rulesWHOISD + "'" );
      }
      String[] transports = Transport.transportTitles;
      logger.debug( "WHOISDServlet.applyTerm() line359" );
      if (rulesWHOISD.size()>0)
      {
          logger.debug( "WHOISDServlet.applyTerm() (rulesWHOISD.size()>0) size=" + rulesWHOISD.size() );
          try {
              for (byte i =0 ;i<transports.length;i++) {
                logger.debug( "WHOISDServlet.applyTerm() for rule '" + transports[i] + "'" );
                ruleWHOISD = (Rule)rulesWHOISD.get(transports[i]);
                RuleState curRuleState = ruleManagerWrap.getRuleStateAndLock(service,transports[i]);
                logger.debug( "WHOISDServlet.applyTerm() curRuleState='" + curRuleState + "'" );
                if (ruleWHOISD == null) {
                  if (curRuleState.getExists()) {
                    ruleManagerWrap.removeRule(service,transports[i]);
                  }
                  continue;
                }

                String ruleSystemId = composePath(GW_LOCATION_RULES_FOLDER, appContext)+"/"+ transports[i] + "/"+ruleWHOISD.getId().toString();
                try {
                  SAXParserImpl.parseRule( ruleManagerWrap.getRuleContentAsString(ruleWHOISD), ruleSystemId, transports[i] );
                } catch(WHOISDException e) {
                  throw new WHOISDException(composeErrorMessage( ruleWHOISD,termAsList, e.getMessage(), e.getLineNumber()) );
                }
                logger.debug( "WHOISDServlet.applyTerm() ruleWHOISD != null)" + (ruleWHOISD != null) + "curRuleState.getExists()='" + curRuleState.getExists() + "'" );
//                if( ruleWHOISD != null && curRuleState.getExists() ) {
                if( ruleWHOISD != null && curRuleState.getExists() && appContext.getRuleManager().checkRuleFileExists( service, transports[i] ) ) {
                   logger.debug( "WHOISDServlet.applyTerm() curRuleState.getExists()" );
                   LinkedList error = ruleManagerWrap.updateRule_whoisd( ruleWHOISD, service, transports[i] );
                   logger.debug( "WHOISDServlet.applyTerm() curRuleState.getExists(), done for service=" + service + ", transport=" + transports[i] +
                           " with error='" + error + "'" );
                   if (error != null && error.size()>0)
                     throw new WHOISDException(composeErrorMessage(ruleWHOISD,termAsList, (String)error.get(0) + " Line in term - {0}.", Integer.parseInt((String)error.get(1))) );
//                } else if (ruleWHOISD != null && !curRuleState.getExists()) {
                } else if (ruleWHOISD != null && !appContext.getRuleManager().checkRuleFileExists( service, transports[i] )) {    
                   logger.debug( "WHOISDServlet.applyTerm() (!curRuleState.getExists())" );
                   LinkedList error = ruleManagerWrap.AddRule_whoisd(ruleWHOISD,service, transports[i]);
                   logger.debug( "WHOISDServlet.applyTerm() (!curRuleState.getExists()) error='" + error + "'");
                   if (error != null && error.size()>0)
                     throw new WHOISDException(composeErrorMessage(ruleWHOISD,termAsList, (String)error.get(0) + " Line in term - {0}.", Integer.parseInt((String)error.get(1))) );
                }
              }
              ruleManagerWrap.applyTerm(RuleManagerWrapper.TERM_COMMIT);
          } catch (Exception e) {
            ruleManagerWrap.applyTerm(RuleManagerWrapper.TERM_ROLLBACK);
            throw e;
          } finally {
            ruleManagerWrap.unlockRules();
          }
      }
    }

    private void debugFlushBos(ByteArrayOutputStream bos) {
        try {
            FileOutputStream fos = new FileOutputStream(System.currentTimeMillis() + ".bos1.log");
            fos.write(bos.toByteArray());
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
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
     //System.out.println("!!!!!!!!!!!!!!! startindex="+startindex);
     //System.out.println("+++++++++++++++++++++++++++++++");
     for (Iterator iter=termAsList.iterator();iter.hasNext();){
        //System.out.println(iter.next());
     }
     //System.out.println("+++++++++++++++++++++++++++++++");

     //System.out.println("-------------------------------");
     for (Iterator iter=ruleBodyWithoutHeader.iterator();iter.hasNext();){
        //System.out.println(iter.next());
     }
     //System.out.println("-------------------------------");
     //System.out.println("offset: " + offset);

     //System.out.println("e.getLineNumber() - ruleWHOISD.getOffset() - ruleHeaderLength: " + (linenumber - ruleWHOISD.getWhoisdPartOffset() - ruleHeaderLength));
     }

     Object linenumberinterm = null;
     if (offset!=-1)
      linenumberinterm = new Integer(offset + linenumber - ruleWHOISD.getWhoisdPartOffset() - ruleHeaderLength - 1);
     else linenumberinterm = new String("unknown");
     return MessageFormat.format(errormessage, new Object[]{linenumberinterm});
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
      while( (bytes=is.read())!=-1 ) {
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

  private void SendResult(WHOISDResult result, HttpServletResponse resp, String contentType) throws IOException
  {
      logger.debug( "WHOISDServlet.SendResult() start" );
    int length = 0;
    if (!resp.containsHeader("status"))
    {
      logger.debug( "WHOISDServlet.SendResult() Request is served successfully" );
      resp.setHeader("status","ok");
      if (result != null && result.result != null && result.result.length > 0) {
          length = result.result.length;
        if( contentType.equals("") ) contentType = CONTENT_TYPE_TXT;
          logger.debug( "WHOISDServlet.SendResult() \ncontentType='" + contentType + "'\nsetCharacterEncoding='" + Functions.getLocaleEncoding() + "'" );
        resp.setContentType(contentType);
        resp.setCharacterEncoding(Functions.getLocaleEncoding());
      }
    }
    else {
      String error = (result != null && result.error != null) ? result.error : null;
      logger.error("WHOISDServlet.SendResult() Request is not served, reason: " + error);
    }
    if( result.error==null ){
        logger.debug( "WHOISDServlet.SendResult() error==null" );
        resp.setContentLength( length );
        OutputStream out = null;
        try {
            out = resp.getOutputStream();
            out.write(result.result);
            out.flush();
        }
        finally {
            if (out!=null) out.close();
        }
    } else {
        logger.debug( "WHOISDServlet.SendResult() error" );
        ServletOutputStream serOut = null;
        try{
            serOut = resp.getOutputStream();
            serOut.println( result.error );
            serOut.flush();
        }finally{
            if(serOut!=null) serOut.close();
        }
    }

  }

  private int getRequestId(HttpServletRequest req)
  {
    String[] parsedURI = req.getRequestURI().split("/");
    String requestedFile = parsedURI[parsedURI.length-1];
      logger.debug( "WHOISDServlet.getRequestId() requestedFile='" + requestedFile + "'" );
      logger.debug( "WHOISDServlet.getRequestId() WHOISDRequest.getId(requestedFile)='" + WHOISDRequest.getId(requestedFile) + "'" );
    return WHOISDRequest.getId(requestedFile);
  }

  private WHOISDResult getRule(HttpServletRequest req, SCAGAppContext appContext) throws WHOISDException , Exception {
      logger.debug( "WHOISDServlet.getRule() start" );
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
    File ruleFile = appContext.getRuleManager().composeRuleFile(transport, serviceId.toString());
    if (!ruleFile.exists()) throw new WHOISDException("There is no rule for service with id = " + serviceId + " and transport "+ transport);
    return loadFile(ruleFile);
  }

  private WHOISDResult loadXml(String filepath) throws Exception {
    File filetoread = new File(filepath);
    return loadFile(filetoread);
  }

  private WHOISDResult loadFile(File file) throws IOException {
    InputStream input = null;
    WHOISDResult result = new WHOISDResult();
    try {
      input = new BufferedInputStream(new FileInputStream(file));
      int size = input.available();
      result.result = new byte[size];
      Functions.readBuffer(input, result.result, size);
    }
    catch (IOException e) {
      result = addError("WHOISDServlet can not load file:" + e.getMessage());
    }
    finally {
      if (input != null) input.close();
    }
    return result;
  }

  private LinkedList loadFileToList(File file) throws Exception {
    LinkedList li = new LinkedList();
    InputStream in = null;
    BufferedReader br = null;
    try {
      in = new FileInputStream(file);
      br = new BufferedReader(new InputStreamReader(in, Functions.getLocaleEncoding()));
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

  private WHOISDResult getSchema(int id, String xmlPath) throws Exception
  {
    RuleManager.Schema schema = (RuleManager.Schema)schemaCash.get(xmlPath);
    File filetoread = new File(xmlPath);
    File schemaFile = new File(filetoread.getParent(), WHOISDRequest.getSchemaName(id));
    if (schema==null || schemaFile.lastModified() > schema.lastmodified) {
       schema = new RuleManager.Schema(schemaFile.lastModified(),loadFileToList(schemaFile));
       schemaCash.put(xmlPath,schema);
    }

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    PrintWriter pw = new PrintWriter(bos);
    for (Iterator i = schema.schemaContent.iterator(); i.hasNext();) {
      String s = (String)i.next();
      pw.println(s);
    }
    pw.flush(); pw.close();
    bos.flush(); bos.close();

    WHOISDResult result = new WHOISDResult();
    result.result = bos.toByteArray();
    return result;
  }

  private void validateTransportParameter(String transport) throws WHOISDException{
      logger.debug( "WHOISDServlet.validateTransportParameter() start" );
    String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
     if (transport.equals(transports[i])) return;
    }
    throw new WHOISDException("transport parameter '"+transport+"' doesn't correspond to enumeration: "+Arrays.asList(transports));
  }
}