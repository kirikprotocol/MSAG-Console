package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;
import java.util.Collection;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 30.11.2010
 * Time: 14:12:49
 */
interface UserDirResolver {

  public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException;

  public Collection<File> getAllUserLocalDirs(String login) throws AdminException;

  public FileResource getConnection(User user, UserCPsettings ucps) throws AdminException;

}
