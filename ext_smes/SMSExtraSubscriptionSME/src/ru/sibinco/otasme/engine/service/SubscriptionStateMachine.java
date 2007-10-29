package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.AbstractStateMachine;
import com.eyeline.sme.utils.statemachine.Event;
import com.eyeline.sme.utils.worker.IterativeWorker;

import java.util.*;

import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.otasme.utils.ConnectionPoolFactory;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import snaq.db.ConnectionPool;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

public class SubscriptionStateMachine extends AbstractStateMachine implements SubscriptionState {

  private static final Category log = Category.getInstance("STATE MACHINE");

  private final Map currentStates = new HashMap(); // States for abonents

  private final ConnectionPool pool;

  private final AbstractSubscriptionState subscribeAbonentState;
  private final AbstractSubscriptionState unsubscribeAbonentState;
  private final AbstractSubscriptionState handleSubscrWTSResponseState;
  private final AbstractSubscriptionState handleUnsubscrWTSResponseState;

  public SubscriptionStateMachine(SMPPOutgoingQueue outQueue, SmsCentersList smsCenters) {
    this.pool = ConnectionPoolFactory.createConnectionPool("Subscription", 10, 20000);
    pool.init(1);

    // Init states
    subscribeAbonentState = new SubscribeAbonentState(this, pool, outQueue, smsCenters);
    unsubscribeAbonentState = new UnsubscribeAbonentState(this, pool, outQueue, smsCenters);
    handleSubscrWTSResponseState = new HandleWTSResponseState(this, pool, outQueue, smsCenters, true);
    handleUnsubscrWTSResponseState = new HandleWTSResponseState(this, pool, outQueue, smsCenters, false);
    // Start states registry cleaner
    new StateRegistryCleaner().start();
  }

  private SubscriptionState getCurrentStateForAbonent(String abonentAddress) {
    synchronized(currentStates) {
      final StateInfo stateInfo = (StateInfo)currentStates.remove(abonentAddress);
      return (stateInfo == null) ? null : stateInfo.state;
    }
  }

  private SubscriptionState getCurrentStateForAbonent(String abonentAddress, SubscriptionState defaultState) {
    SubscriptionState state = getCurrentStateForAbonent(abonentAddress);
    return state == null ? defaultState : state;
  }

  private void setCurrentStateForAbonent(String abonentAddress, AbstractSubscriptionState state) {
    synchronized(currentStates) {
      currentStates.put(abonentAddress, new StateInfo(state));
    }
  }

  private void removeStateForAbonent(String abonentAddress) {
    synchronized(currentStates) {
      currentStates.remove(abonentAddress);
    }
  }


  public synchronized int subscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    try {
      log.info("subscr req: srcaddr=" + abonentAddress + "; external=" + isAbonentExternal);

      final SubscriptionState state = getCurrentStateForAbonent(abonentAddress, subscribeAbonentState);
      return state.subscribeAbonent(abonentAddress, isAbonentExternal);

    } catch (Throwable e) {
      log.error("subscr err for " + abonentAddress,e);
      return STATUS_SYSTEM_ERROR;
    }
  }


  public synchronized int unsubscribeAbonent(String abonentAddress, boolean isAbonentExternal) {
    try {
      log.info("unsubscr req: srcaddr=" + abonentAddress + "; external=" + isAbonentExternal);

      final SubscriptionState state = getCurrentStateForAbonent(abonentAddress, unsubscribeAbonentState);
      return state.unsubscribeAbonent(abonentAddress, isAbonentExternal);

    } catch (Throwable e) {
      log.error("unsubscr err for " + abonentAddress,e);
      return STATUS_SYSTEM_ERROR;
    }
  }


  public synchronized int handleWTSResponse(String abonentAddress, long code, int status) {
    try {
      final SubscriptionState state = getCurrentStateForAbonent(abonentAddress);
      if (state == null) {
        log.error("Can't find state for " + abonentAddress);
        return STATUS_SYSTEM_ERROR;
      }
      return state.handleWTSResponse(abonentAddress, code, status);

    } catch (Throwable e) {
      log.error("handle WTS response error for " + abonentAddress,e);
      return STATUS_SYSTEM_ERROR;
    }
  }


  public synchronized int handleWTSSendError(String abonentAddress) {
    try {
      final SubscriptionState state = getCurrentStateForAbonent(abonentAddress);
      if (state == null) {
        log.error("Can't find state for " + abonentAddress);
        return STATUS_SYSTEM_ERROR;
      }
      return state.handleWTSSendError(abonentAddress);

    } catch (Throwable e) {
      log.error("handle WTS response error for " + abonentAddress,e);
      return STATUS_SYSTEM_ERROR;
    }
  }


  public void castEvent(Event event) {
    try {
      if (event == null || !(event instanceof StateEvent))
        return;

      final StateEvent stateEvent = (StateEvent)event;
      switch(stateEvent.getType()) {
        case StateEvent.TYPE_SET_WTS_SUBSCRIPTION_STATE:
          setCurrentStateForAbonent(stateEvent.getAbonentAddress(), handleSubscrWTSResponseState);
          break;
        case StateEvent.TYPE_SET_WTS_UNSUBSCRIPTION_STATE:
          setCurrentStateForAbonent(stateEvent.getAbonentAddress(), handleUnsubscrWTSResponseState);
          break;
        case StateEvent.TYPE_REMOVE_STATE:
          removeStateForAbonent(stateEvent.getAbonentAddress());
          break;
        default:
          log.error("Unexpected event type = " + stateEvent.getType());
      }

    } catch (Throwable e) {
      log.error("Can't cast event", e);
    }
  }


  private static class StateInfo {
    private AbstractSubscriptionState state;
    private long createTime;

    public StateInfo(AbstractSubscriptionState state) {
      this.state = state;
      this.createTime = System.currentTimeMillis();
    }
  }


  private class StateRegistryCleaner extends IterativeWorker {

    public StateRegistryCleaner() {
      super(SmeProperties.SessionsRegistry.CLEAN_INTERVAL);
    }

    protected void iterativeWork() {
      final long now = System.currentTimeMillis();
      synchronized (currentStates) {
        final List sessions2remove = new ArrayList();
        Map.Entry entry;
        String abonentAddress;
        StateInfo stateInfo;
        for (Iterator iter = currentStates.entrySet().iterator(); iter.hasNext(); ) {
          entry = (Map.Entry)iter.next();
          abonentAddress = (String)entry.getKey();
          stateInfo = (StateInfo)entry.getValue();

          if (now - stateInfo.createTime > SmeProperties.SessionsRegistry.SESSION_IDLE_TIME) {
            log.error("Remove stateInfo with id=" + abonentAddress + " by timeout");
            stateInfo.state.doBeforeRemove(abonentAddress);
            sessions2remove.add(abonentAddress);
          }
        }

        for (Iterator iter = sessions2remove.iterator(); iter.hasNext();)
          currentStates.remove(iter.next());

      }
    }

    protected void stopCurrentWork() {
    }
  }
}
