/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 16:56:09
 */
package ru.novosoft.smsc.jsp.util.tables;


public interface DataSource
{
  QueryResultSet query(Query query_to_run);
}
