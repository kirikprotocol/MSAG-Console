package ru.sibinco.lib.backend.util.conpool;

import java.sql.Connection;
import java.sql.SQLException;


public class NSReusableConnection implements java.sql.Connection
{
//  public static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance("ru.novosoft.util.conpool");
  private Connection con = null;
  private NSPooledConnection pcon = null;

  public NSReusableConnection(final Connection con, final NSPooledConnection pcon)
  {
    this.con = con;
    this.pcon = pcon;
  }

  // just makes pooled connection free
  public void close() throws SQLException
  {
    pcon.free();
  }

  // really close DB connection
  public void realClose() throws SQLException
  {
    con.close();
  }

  public java.sql.Statement createStatement() throws SQLException
  {
    return con.createStatement();
  }

  public java.sql.PreparedStatement prepareStatement(final String A) throws SQLException
  {
    return con.prepareStatement(A);
  }

  public boolean getAutoCommit() throws SQLException
  {
    return con.getAutoCommit();
  }

  public java.sql.CallableStatement prepareCall(final String A) throws SQLException
  {
    return con.prepareCall(A);
  }

  public String nativeSQL(final String A) throws SQLException
  {
    return con.nativeSQL(A);
  }

  public void setAutoCommit(final boolean A) throws SQLException
  {
    con.setAutoCommit(A);
  }

  public boolean isClosed() throws SQLException
  {
    return con.isClosed();
  }

  public void commit() throws SQLException
  {
    con.commit();
  }

  public void rollback() throws SQLException
  {
    con.rollback();
  }

  public void setCatalog(final String A) throws SQLException
  {
    con.setCatalog(A);
  }

  public java.sql.DatabaseMetaData getMetaData() throws SQLException
  {
    return con.getMetaData();
  }

  public void setReadOnly(final boolean A) throws SQLException
  {
    con.setReadOnly(A);
  }

  public boolean isReadOnly() throws SQLException
  {
    return con.isReadOnly();
  }

  public java.sql.SQLWarning getWarnings() throws SQLException
  {
    return con.getWarnings();
  }

  public String getCatalog() throws SQLException
  {
    return con.getCatalog();
  }

  public void setTransactionIsolation(final int A) throws SQLException
  {
    con.setTransactionIsolation(A);
  }

  public int getTransactionIsolation() throws SQLException
  {
    return con.getTransactionIsolation();
  }

  public java.sql.CallableStatement prepareCall(final String A, final int B, final int C) throws SQLException
  {
    return con.prepareCall(A, B, C);
  }

  public void clearWarnings() throws SQLException
  {
    con.clearWarnings();
  }

  public java.sql.Statement createStatement(final int A, final int B) throws SQLException
  {
    return con.createStatement(A, B);
  }

  public java.sql.PreparedStatement prepareStatement(final String A, final int B, final int C) throws SQLException
  {
    return con.prepareStatement(A, B, C);
  }

  public int getHoldability() throws SQLException
  {
    return con.getHoldability();
  }

  public java.util.Map getTypeMap() throws SQLException
  {
    return con.getTypeMap();
  }

  public void setTypeMap(final java.util.Map A) throws SQLException
  {
    con.setTypeMap(A);
  }

  public void setHoldability(final int A) throws SQLException
  {
    con.setHoldability(A);
  }

  public java.sql.Savepoint setSavepoint() throws SQLException
  {
    return con.setSavepoint();
  }

  public java.sql.Savepoint setSavepoint(final String A) throws SQLException
  {
    return con.setSavepoint(A);
  }

  public void rollback(final java.sql.Savepoint A) throws SQLException
  {
    con.rollback(A);
  }

  public void releaseSavepoint(final java.sql.Savepoint A) throws SQLException
  {
    con.releaseSavepoint(A);
  }

  public java.sql.Statement createStatement(final int A, final int B, final int C) throws SQLException
  {
    return con.createStatement(A, B, C);
  }

  public java.sql.PreparedStatement prepareStatement(final String A, final int B, final int C, final int D) throws SQLException
  {
    return con.prepareStatement(A, B, C, D);
  }

  public java.sql.CallableStatement prepareCall(final String A, final int B, final int C, final int D) throws SQLException
  {
    return con.prepareCall(A, B, C, D);
  }

  public java.sql.PreparedStatement prepareStatement(final String A, final int B) throws SQLException
  {
    return con.prepareStatement(A, B);
  }

  public java.sql.PreparedStatement prepareStatement(final String A, final int[] B) throws SQLException
  {
    return con.prepareStatement(A, B);
  }

  public java.sql.PreparedStatement prepareStatement(final String A, final String[] B) throws SQLException
  {
    return con.prepareStatement(A, B);
  }
}

