package mobi.eyeline.informer.admin.contentprovider.resources;

import it.sauronsoftware.ftp4j.FTPClient;
import it.sauronsoftware.ftp4j.FTPConnector;
import it.sauronsoftware.ftp4j.FTPFile;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class FTPResource extends FileResource {

  private FTPClient ftp;

  private final String host;
  private final Integer port;
  private final String login;
  private final String password;
  private final String remoteDir;
  private final boolean passiveMode;

  FTPResource(String host, Integer port, String login, String password, String remoteDir, boolean passiveMode) {
    this.host = host;
    this.port = port;
    this.login = login;
    this.password = password;
    this.remoteDir = remoteDir;
    this.passiveMode = passiveMode;
  }

  @Override
  public void open() throws AdminException {
    try{
      ftp = new FTPClient();
      ftp.setConnector(new FtpConnectorWrapper(ftp.getConnector()));
      ftp.setPassive(passiveMode);
      ftp.setType(FTPClient.TYPE_BINARY);
      ftp.setCharset("cp1251");
      if(port == null || port == 0) {
        ftp.connect(host);
      } else {
        ftp.connect(host, port);
      }
      ftp.login(login, password);
      ftp.changeDirectory(remoteDir);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  public List<String> listFiles() throws AdminException {
    List<String> result = new LinkedList<String>();
    try{
      FTPFile[] fs = ftp.list();
      for(FTPFile f : fs) {
        String name = f.getName();
        result.add(name);
      }
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
    return result;
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
    try{
      ftp.download(path, os, 0, null);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    try{
      ftp.rename(fromPath, toPath);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void remove(String path) throws AdminException {
    try{
      ftp.deleteFile(path);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
    try{
      ftp.upload(toPath, is, 0, 0, null);
    }catch (Exception e){
      throw new ContentProviderException("connectionError",e);
    }
  }

  @Override
  public void close() throws AdminException {
    if(ftp != null) {
      try{
        ftp.disconnect(true);
      } catch (Exception e) {
        throw new ContentProviderException("connectionError",e);
      }
    }
  }

  public String toString() {
    return "ftp://" + login + "@" + host + ':' + port + "/" + remoteDir;
  }

  private static class FtpConnectorWrapper implements FTPConnector{

    private static final int timeout = 30000;

    private final FTPConnector connector;

    private FtpConnectorWrapper(FTPConnector connector) {
      this.connector = connector;
    }

    public Socket connectForCommunicationChannel(String s, int i) throws IOException {
      Socket socket = connector.connectForCommunicationChannel(s, i);
      socket.setSoTimeout(timeout);
      return socket;
    }

    public Socket connectForDataTransferChannel(String s, int i) throws IOException {
      Socket socket = connector.connectForDataTransferChannel(s, i);
      socket.setSoTimeout(timeout);
      return socket;
    }
  }
}
