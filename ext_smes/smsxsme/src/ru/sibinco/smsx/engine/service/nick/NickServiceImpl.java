package ru.sibinco.smsx.engine.service.nick;

import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;
import com.eyeline.sme.smpp.OutgoingQueue;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

class NickServiceImpl implements NickService {

  private final NickServiceConfig config;
  private final NickProcessor processor;

  public NickServiceImpl(String configDir, OutgoingQueue outQueue) {
    this.config = new NickServiceConfig(configDir);
    this.processor = new NickProcessor(config, outQueue);
  }

  public void execute(NickRegisterCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(NickSendMessageCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(NickUnregisterCmd cmd) {
    processor.execute(cmd);
  }

  public void startService() {
  }

  public void stopService() {
  }


}
