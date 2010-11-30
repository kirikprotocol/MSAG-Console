package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 16:59:20
 */
interface ContentProviderConnection {

  public void connect() throws AdminException;

  public List<String> listCSVFiles() throws AdminException;

  public void get(String path, File localFile)  throws AdminException;

  public void rename(String fromPath, String toPath) throws AdminException;

  public void put(File localFile, String toPath) throws AdminException;

  public void close() throws AdminException;

}
