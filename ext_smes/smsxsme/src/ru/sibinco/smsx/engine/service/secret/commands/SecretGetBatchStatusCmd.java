package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 18.11.2008
 */
public class SecretGetBatchStatusCmd extends Command {

  public static final int BATCH_STATUS_PROCESSED = -1;
  public static final int BATCH_STATUS_ERROR = -2;
  public static final int BATCH_STATUS_UNKNOWN = -3;

  private String batchId;

  public SecretGetBatchStatusCmd(String batchId) {
    this.batchId = batchId;
  }

  public String getBatchId() {
    return batchId;
  }

  public void setBatchId(String batchId) {
    this.batchId = batchId;
  }

  public interface Receiver {
    public int execute(SecretGetBatchStatusCmd cmd) throws CommandExecutionException;
  }
}
