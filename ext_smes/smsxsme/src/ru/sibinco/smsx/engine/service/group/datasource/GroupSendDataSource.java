package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 15.07.2008
 */

public interface GroupSendDataSource {

  public void setStatus(long msgKey, MessageStatus status) throws DataSourceException;
  public boolean containsStatus(long msgKey) throws DataSourceException;
  public MessageStatus getStatus(long msgKey) throws DataSourceException;

  public void shutdown();

  public enum MessageStatus {
    ACCEPTED(0), DELIVERED(1), OWNER_NOT_FOUND(2), LIST_NOT_FOUND(3), SYS_ERROR(4);

    int id;

    MessageStatus(int id) {
      this.id = id;
    }

    int getId() {
      return id;
    }

    static MessageStatus getById(int id) {
      switch(id) {
        case 0: return ACCEPTED;
        case 1 : return DELIVERED;
        case 2 : return OWNER_NOT_FOUND;
        case 3 : return LIST_NOT_FOUND;
        case 4 : return SYS_ERROR;
        default: return null;
      }
    }
  }
}
