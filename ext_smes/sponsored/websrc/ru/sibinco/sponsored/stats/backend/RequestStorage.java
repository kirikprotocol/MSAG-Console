package ru.sibinco.sponsored.stats.backend;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class RequestStorage {

  private final Map requests = new HashMap();

  private final File requestsDir;

  RequestStorage(File requestsDir) throws StatisticsException{
    this.requestsDir = requestsDir;
    if(!requestsDir.exists()) {
      if(!requestsDir.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+requestsDir.getAbsolutePath());
      }
    }else {
      loadFromFS();
    }
  }

  private void loadFromFS() throws StatisticsException{
    File[] fs = requestsDir.listFiles();
    for(int i=0; i<fs.length; i++) {
      File f = fs[i];
      if(!f.isDirectory()) {
        continue;
      }
      SponsoredRequest r = loadRequest(f);
      if(r == null) {
        continue;
      }
      if(!isStatusFinished(r.getStatus())) {
        r.setStatus(SponsoredRequest.Status.IN_PROCESS);
      }
      requests.put(new Integer(r.getId()), r);
    }
  }

  File buildFile(SponsoredRequest request) {
    StringBuffer sb = new StringBuffer(25).append(request.getId()).
        append(File.separatorChar).append("request.xml");
    return new File(requestsDir, sb.toString());
  }



  public synchronized SponsoredRequest getRequest(int requestId) {
    SponsoredRequest r = (SponsoredRequest)requests.get(new Integer(requestId));
    return r == null ? null : r.copy();
  }



  public synchronized void createRequest(SponsoredRequest request) throws StatisticsException {
    saveRequest(request.copy());
    requests.put(new Integer(request.getId()), request);
  }


  public synchronized void removeRequest(int requestId) throws StatisticsException {
    SponsoredRequest request = (SponsoredRequest)requests.remove(new Integer(requestId));
    if(request != null) {
      File f = buildFile(request);
      if(f.exists() && !f.delete()) {
        throw new StatisticsException("Can't remove request file: "+f.getAbsolutePath());
      }
    }
  }

  public synchronized List getRequests() {
    List result = new ArrayList(requests.size());
    Iterator i = requests.values().iterator();
    while(i.hasNext()) {
      result.add(((SponsoredRequest)i.next()).copy());
    }
    return result;
  }

  public synchronized List getSponsoredRequests() {
    List result = new ArrayList(requests.size());
    Iterator i = requests.values().iterator();
    while(i.hasNext()) {
      SponsoredRequest r = (SponsoredRequest)i.next();
      result.add(r.copy());
    }
    return result;
  }

  public synchronized void changeStatus(int requestId, SponsoredRequest.Status status) throws StatisticsException {
    SponsoredRequest request = (SponsoredRequest)requests.get(new Integer(requestId));
    if(request == null) {
      throw new StatisticsException("Request is not found with id="+requestId);
    }
    request.setStatus(status);
    saveRequest(request);
  }

  public synchronized void setError(int requestId, String error) throws StatisticsException {
    SponsoredRequest request = (SponsoredRequest)requests.get(new Integer(requestId));
    if(request == null) {
      throw new StatisticsException("Request is not found with id="+requestId);
    }
    request.setError(error);
    saveRequest(request);
  }

  private final DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();

  SponsoredRequest loadRequest(File f) throws StatisticsException{
    InputStream is = null;
    File[] list = f.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith("request.xml");
      }
    });
    if(list == null || list.length == 0) {
      return null;
    }
    try{
      is = new FileInputStream(list[0]);
      Document d = documentBuilderFactory.newDocumentBuilder().parse(is);
      NodeList nl = d.getElementsByTagName("request");
      if(nl.getLength()>0) {
        SponsoredRequest r = new SponsoredRequest();
        r.load((Element)nl.item(0));
        return r;
      }
      return null;
    }catch (Exception e){
      throw new StatisticsException(e);
    } finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException ignored){}
      }
    }
  }


  void saveRequest(SponsoredRequest r) throws StatisticsException {
    File f = buildFile(r);
    File p = f.getParentFile();
    if(p != null && !p.exists() && !p.mkdirs()) {
      throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
    }
    try{
      Document d = documentBuilderFactory.newDocumentBuilder().newDocument();
      Element el = d.createElement("request");
      d.appendChild(el);
      r.save(el, d);

      OutputStream out = null;
      try{
        out = new FileOutputStream(f);
        Source source = new DOMSource(d.getDocumentElement());
        Result result = new StreamResult(out);
        Transformer xformer = TransformerFactory.newInstance().newTransformer();
        xformer.setOutputProperty(OutputKeys.INDENT, "yes");
        xformer.transform(source, result);
      }finally {
        if(out != null) {
          try{
            out.close();
          }catch (IOException ignored){}
        }
      }
    }catch (Exception e) {
      throw new StatisticsException(e);
    }
  }

  private static boolean isStatusFinished(SponsoredRequest.Status st) {
    return st == SponsoredRequest.Status.READY || st == SponsoredRequest.Status.CANCELED ||  st == SponsoredRequest.Status.ERROR ;
  }

}
