package ru.novosoft.smsc.jsp.util.tables;

/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 16:56:09
 */

import ru.novosoft.smsc.admin.AdminException;


public interface DataSource
{
  QueryResultSet query(Query query_to_run) throws AdminException;
}
