/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:37:32
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;


public class SubjectDataSource extends AbstractDataSourceImpl
{
  private static final String[] columnNames = {"Name", "Default SME", "Masks"};

  public SubjectDataSource()
  {
    super(columnNames);
  }

  public void add(Subject subj)
  {
    super.add(new SubjectDataItem(subj));
  }

  public void remove(Subject subj)
  {
    super.remove(new SubjectDataItem(subj));
  }

  public QueryResultSet query(SubjectQuery query_to_run)
  {
    return super.query(query_to_run);
  }
}
