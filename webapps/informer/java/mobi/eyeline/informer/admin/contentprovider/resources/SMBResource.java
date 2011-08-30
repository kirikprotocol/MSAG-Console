package mobi.eyeline.informer.admin.contentprovider.resources;

import jcifs.smb.NtlmPasswordAuthentication;
import jcifs.smb.SmbFile;
import jcifs.smb.SmbNamedPipe;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderException;

import java.io.*;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class SMBResource extends FileResource {

  private SmbNamedPipe smb;

  private final String host;
  private final Integer port;
  private final String login;
  private final String password;
  private final String remoteDir;
  private final String domain;

  SMBResource(String host, Integer port, String login, String password, String remoteDir, String domain) {
    this.host = host;
    this.port = port;
    this.login = login;
    this.password = password;
    this.remoteDir = remoteDir;
    this.domain = domain != null && domain.length() == 0 ? null : domain;
  }

  @Override
  public void open() throws AdminException {
    StringBuilder url = new StringBuilder().append("smb://").
        append(host);
    if (port != null && port != 0) {
      url.append(':').append(port);
    }

    if (!remoteDir.startsWith("/")) {
      url.append('/');
    }
    url.append(remoteDir);
    if (!remoteDir.endsWith("/")) {
      url.append('/');
    }

    try {
      if ((login == null || login.length() == 0) &&
          (password == null || password.length() == 0)) {
        smb = new SmbNamedPipe(url.toString(), SmbNamedPipe.PIPE_TYPE_RDWR | SmbNamedPipe.PIPE_TYPE_CALL);
      } else {
        smb = new SmbNamedPipe(url.toString(), SmbNamedPipe.PIPE_TYPE_RDWR | SmbNamedPipe.PIPE_TYPE_CALL,
            new NtlmPasswordAuthentication(domain, login, password));
      }
    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public List<String> listFiles() throws AdminException {
    List<String> result = new LinkedList<String>();
    try {
      String[] fs = smb.list();
      Collections.addAll(result, fs);
    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
    return result;
  }

  @Override
  public void get(String path, OutputStream os) throws AdminException {
    InputStream is = null;
    try {
      SmbFile file = new SmbFile(smb, path);

      is = new BufferedInputStream(file.getInputStream());
      byte[] buffer = new byte[1024];
      int readed;
      while ((readed = is.read(buffer)) > 0) {
        os.write(buffer, 0, readed);
      }

    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException ignored) {
        }
      }
    }
  }

  @Override
  public void rename(String fromPath, String toPath) throws AdminException {
    try {
      new SmbFile(smb, fromPath).renameTo(new SmbFile(smb, toPath));
    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public void remove(String path) throws AdminException {
    try {
      new SmbFile(smb, path).delete();
    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    }
  }

  @Override
  public void put(InputStream is, String toPath) throws AdminException {
    OutputStream os = null;
    try {
      SmbFile remote = new SmbFile(smb, toPath);
      remote.createNewFile();

      os = new BufferedOutputStream(remote.getOutputStream());
      byte[] buffer = new byte[1024];
      int readed;
      while ((readed = is.read(buffer)) > 0) {
        os.write(buffer, 0, readed);
      }

    } catch (Exception e) {
      throw new ContentProviderException("connectionError", e);
    } finally {
      if (os != null) {
        try {
          os.close();
        } catch (IOException ignored) {
        }
      }
    }

  }

  @Override
  public void close() throws AdminException {
  }

  public String toString() {
    return "smb://" + login + '@' + host + ':' + port + '/' + remoteDir;
  }
}
