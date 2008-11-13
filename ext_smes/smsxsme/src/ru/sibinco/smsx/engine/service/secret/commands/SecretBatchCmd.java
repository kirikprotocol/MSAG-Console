package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

import java.io.InputStream;


/**
 * User: artem
 * Date: 31.10.2008
 */
public class SecretBatchCmd extends Command {

  private String sourceAddress;
  private InputStream destinations;
  private String message;
  private int destAddressSubunit;

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public InputStream getDestinations() {
    return destinations;
  }

  public void setDestinations(InputStream destinations) {
    this.destinations = destinations;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public int getDestAddressSubunit() {
    return destAddressSubunit;
  }

  public void setDestAddressSubunit(int destAddressSubunit) {
    this.destAddressSubunit = destAddressSubunit;
  }

  public interface Receiver {
    public void execute(SecretBatchCmd cmd) throws CommandExecutionException;
  }
  
}
