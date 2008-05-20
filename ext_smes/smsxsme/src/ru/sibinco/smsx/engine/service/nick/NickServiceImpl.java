package ru.sibinco.smsx.engine.service.nick;

import ru.sibinco.smsx.engine.service.nick.commands.NickSendMessageCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickRegisterCmd;
import ru.sibinco.smsx.engine.service.nick.commands.NickUnregisterCmd;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

class NickServiceImpl implements NickService {

  private final NickServiceConfig conf;
  private final NickProcessor processor;

  public NickServiceImpl(XmlConfig config, OutgoingQueue outQueue) {
    this.conf = new NickServiceConfig(config);
    this.processor = new NickProcessor(conf, outQueue);
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
