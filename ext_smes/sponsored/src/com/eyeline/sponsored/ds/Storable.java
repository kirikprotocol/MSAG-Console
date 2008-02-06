package com.eyeline.sponsored.ds;



/**
 *
 * @author artem
 */
public interface Storable {

  /**
   * Stores storable into data source or update it
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public void save() throws DataSourceException;
  
  /**
   * Stores storable into data source in specified transaction
   * @param tx
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public void save(DataSourceTransaction tx) throws DataSourceException;

  /**
   * Removes storable from data source if exists
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public void remove() throws DataSourceException;
  
  /**
   * Removes storable from data source in 
   * @param tx
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public void remove(DataSourceTransaction tx) throws DataSourceException;
}
