package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author Aleksandr Khalitov
 */
class RequestFileStorage implements RequestStorage {

  private static final Logger logger = Logger.getLogger(RequestFileStorage.class);

  private Map<Integer, Request> requests = new ConcurrentHashMap<Integer, Request>();

  private FileSystem fileSystem;

  private File requestsDir;

  RequestFileStorage(File requestsDir, FileSystem fileSystem) throws InitException{
    this.requestsDir = requestsDir;
    this.fileSystem = fileSystem;
    if(!requestsDir.exists()) {
      if(!requestsDir.mkdirs()) {
        throw new InitException("Can't create dir: "+requestsDir);
      }
    }else {
      loadFromFS();
    }
  }

  private void loadFromFS() throws InitException{
    for(File f : fileSystem.listFiles(requestsDir)) {
      try {
        Request r = loadRequest(f);
        if(!isStatusFinished(r.getStatus())) {
          r.setStatus(DeliveriesRequest.Status.NEW);
        }
        requests.put(r.getId(), r);
      } catch (Exception e) {
        logger.error(e, e);
      }
    }
  }

  static String buildFileName(Request request) {
    return new StringBuilder(25).append(request.getType()).append('.').append(request.getId()).append(".xml").toString();
  }


  @Override
  public Request getRequest(int requestId) throws AdminException {
    Request r = requests.get(requestId);
    return r.copy();
  }


  @Override
  public synchronized void createRequest(Request request) throws AdminException {
    try {
      saveRequest(new File(requestsDir, buildFileName(request)), request.copy());
    } catch (Exception e) {
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
    requests.put(request.getId(), request);
  }

  @Override
  public synchronized void removeRequest(int requestId) throws AdminException {
    Request request = requests.remove(requestId);
    if(request != null) {
      fileSystem.delete(new File(requestsDir, buildFileName(request)));
    }
  }

  @Override
  public Collection<Request> getRequests() throws AdminException {
    Collection<Request> result = new ArrayList<Request>(requests.size());
    for(Request r : requests.values()) {
      result.add(r.copy());
    }
    return result;
  }

  public synchronized void changeStatus(int requestId, DeliveriesRequest.Status status) throws AdminException {
    Request request = requests.get(requestId);
    if(request == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    request.setStatus(status);
    try {
      saveRequest(new File(requestsDir, buildFileName(request)), request);
    } catch (Exception e) {
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }

  @Override
  public synchronized void setEndDate(int requestId, Date date) throws AdminException {
    Request request = requests.get(requestId);
    if(request == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    request.setEndDate(date);
    try {
      saveRequest(new File(requestsDir, buildFileName(request)), request);
    } catch (Exception e) {
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }

  @Override
  public synchronized void rename(int requestId, String name) throws AdminException {
    Request request = requests.get(requestId);
    if(request == null) {
      throw new ArchiveException("request_not_found", Integer.toString(requestId));
    }
    request.setName(name);
    try {
      saveRequest(new File(requestsDir, buildFileName(request)), request);
    } catch (Exception e) {
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }


  Request loadRequest(File f) throws XmlConfigException, AdminException, ParseException {

    XmlConfig c = new XmlConfig();

    InputStream is = null;
    try{
      is = fileSystem.getInputStream(f);
      c.load(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException ignored){}
      }
    }

    Request r;
    if(f.getName().startsWith(Request.Type.deliveries.toString())) {
      r = new DeliveriesRequest();
    }else {
      r = new MessagesRequest();
    }
    r.load(c);

    return r;
  }


  void saveRequest(File f, Request r) throws XmlConfigException, AdminException, ParseException {

    XmlConfig c = new XmlConfig();

    r.save(c);

    OutputStream os = null;
    try{
      os = fileSystem.getOutputStream(f, false);
      c.save(os);
    }finally {
      if(os != null) {
        try{
          os.close();
        }catch (IOException ignored){}
      }
    }
  }

  private static boolean isStatusFinished(DeliveriesRequest.Status st) {
    return st == DeliveriesRequest.Status.FINISHED || st == DeliveriesRequest.Status.CANCELED;
  }

}
