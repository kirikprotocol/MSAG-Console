package ru.novosoft.smsc.admin.smsc_service;

import ru.novosoft.smsc.admin.route.RouteList;
import ru.novosoft.smsc.admin.route.SubjectList;
import ru.novosoft.smsc.admin.AdminException;

/**
 * Created by igork
 * Date: Jan 30, 2003
 * Time: 4:04:28 PM
 */
public interface RouteSubjectManager
{
	RouteList getRoutes();

	SubjectList getSubjects();

	void load() throws AdminException;
  void save() throws AdminException;
  void test() throws AdminException;

  void apply() throws AdminException;

	boolean isSmeUsed(String smeId);
}
