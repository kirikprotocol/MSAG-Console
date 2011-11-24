package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.protocol.*;
import ru.novosoft.smsc.util.Address;

import static ru.novosoft.smsc.admin.mcisme.MCIConverter.convert;

/**
 * author: Aleksandr Khalitov
 */
public class MCISmeImpl implements MCISme {

  private final MCISmeClient client;

  public MCISmeImpl(String host, int port) {
    client = new MCISmeClient(host, port);
  }

  public Profile getProfile(Address subscr) throws AdminException{
    GetProfile r = new GetProfile();
    r.setSubscriber(subscr.getSimpleAddress());
    GetProfileResp resp = client.send(r);
    checkResponse(resp);
    return convert(subscr, resp);
  }

  public void saveProfile(Profile profile) throws AdminException {
    SetProfile r = convert(profile);
    SetProfileResp resp = client.send(r);
    checkResponse(resp);
  }

  public Statistics getStats() throws AdminException {
    GetStats r = new GetStats();
    GetStatsResp resp = client.send(r);
    checkResponse(resp);
    return convert(resp);
  }

  public RunStatistics getRunStats() throws AdminException {
    GetRunStats r = new GetRunStats();
    GetRunStatsResp resp = client.send(r);
    checkResponse(resp);
    return convert(resp);
  }

  public Schedule getSchedule(Address subscr) throws AdminException {
    GetSched r = new GetSched();
    r.setSubscriber(subscr.getSimpleAddress());
    GetSchedResp resp = client.send(r);
    checkResponse(resp);
    return convert(resp);
  }

  public void flushStats() throws AdminException  {
    FlushStats r = new FlushStats();
    FlushStatsResp resp = client.send(r);
    checkResponse(resp);
  }





  public boolean isOnline() throws AdminException {
    return client.isConnected();
  }

  public void shutdown() {
    client.shutdown();
  }

  protected static void checkResponse(Response resp) throws AdminException {
    switch (resp.getStatus()) {
      case SubscriberNotFound: throw new MCISmeException("subscr_not_found", resp.getStatus()+"");
      case SystemError: throw new MCISmeException("interaction_error", resp.getStatus() + "");
    }
  }
}
