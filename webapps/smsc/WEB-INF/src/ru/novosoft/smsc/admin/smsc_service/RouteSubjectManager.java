package ru.novosoft.smsc.admin.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteList;
import ru.novosoft.smsc.admin.route.SubjectList;

import java.util.Date;

/**
 * Created by igork
 * Date: Jan 30, 2003
 * Time: 4:04:28 PM
 */
public interface RouteSubjectManager
{
  RouteList getRoutes();

  SubjectList getSubjects();

  void apply() throws AdminException;   // save to primary config (Applied)

  void load() throws AdminException;    // load from primary config (Applied)

  void save() throws AdminException;    // save to temporal config (Saved)

  void restore() throws AdminException; // load from temporal config (Saved)

  Date getRestoreFileDate() throws AdminException; // get modified date of temporal config

  Date getLoadFileDate() throws AdminException; // get modified date of primary config

  void trace() throws AdminException;   // save to traceable config (Traceable)

  int isSmeUsed(String smeId);

  boolean hasSavedConfiguration();
}
