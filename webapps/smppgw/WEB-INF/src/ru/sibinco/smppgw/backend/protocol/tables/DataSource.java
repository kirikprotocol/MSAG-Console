package ru.sibinco.smppgw.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 20:00:02
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.lib.SibincoException;


public interface DataSource
{
  QueryResultSet query(Query query_to_run) throws SibincoException;
}
