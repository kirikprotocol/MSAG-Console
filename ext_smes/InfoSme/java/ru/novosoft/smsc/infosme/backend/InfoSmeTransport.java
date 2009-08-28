package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.*;

import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 24.09.2003
 * Time: 17:21:16
 */
public class InfoSmeTransport extends Service
{
  private static final String SME_COMPONENT_ID = "InfoSme";
  private static final String method_startTaskProcessor_ID = "startTaskProcessor";
  private static final String method_stopTaskProcessor_ID = "stopTaskProcessor";
  private static final String method_isTaskProcessorRuning_ID = "isTaskProcessorRunning";

  private static final String method_startTaskScheduler_ID = "startTaskScheduler";
  private static final String method_stopTaskScheduler_ID = "stopTaskScheduler";
  private static final String method_isTaskSchedulerRuning_ID = "isTaskSchedulerRunning";

  private static final String method_addTask_ID = "addTask";
  private static final String method_removeTask_ID = "removeTask";
  private static final String method_changeTask_ID = "changeTask";

  private static final String method_applyRetryPolicies_ID = "applyRetryPolicies";

  private static final String method_isTaskEnabled_ID = "isTaskEnabled";
  private static final String method_setTaskEnabled_ID = "setTaskEnabled";

  private static final String method_startTasks_ID = "startTasks";
  private static final String method_stopTasks_ID = "stopTasks";
  private static final String method_getGeneratingTasksMethod_ID = "getGeneratingTasksMethod";
  private static final String method_getProcessingTasksMethod_ID = "getProcessingTasksMethod";

  private static final String method_addSchedule_ID = "addSchedule";
  private static final String method_removeSchedule_ID = "removeSchedule";
  private static final String method_changeSchedule_ID = "changeSchedule";

  private static final String method_flushStatistics_ID = "flushStatistics";
  private static final String method_sendSms_ID = "sendSms";

  private static final String SELECT_TASK_MESSAGES_METHOD_ID = "selectTaskMessages";
  private static final String CHANGE_DELIVERY_MESSAGE_INFO_METHOD_ID = "changeDeliveryMessageInfo";
  private static final String DELETE_DELIVERY_MESSAGES_METHOD_ID = "deleteDeliveryMessages";
  private static final String CHANGE_DELIVERY_TEXT_MESSAGE_METHOD_ID = "changeDeliveryTextMessage";
  private static final String SELECT_TASKS_STATISTIC_METHOD_ID = "selectTasksStatistic";
  private static final String ADD_DELIVERY_MESSAGES_METHOD_ID = "addDeliveryMessages";
  private static final String END_DELIVERY_MESSAGE_GENERATION_ID = "endDeliveryMessagesGeneration";
  private static final String ADD_STATISTIC_RECORD_METHOD_ID = "addStatisticRecord";


  private static final SimpleDateFormat mdf = new SimpleDateFormat("ddMMyyyyHHmmss");
  private static final SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHH");

  public InfoSmeTransport(ServiceInfo info, String host, int port)
  {
    super(info, port);
    this.host = host;
  }

  public void startTaskProcessor() throws AdminException
  {
    callMethod(method_startTaskProcessor_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public void stopTaskProcessor() throws AdminException
  {
    callMethod(method_stopTaskProcessor_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public boolean isTaskProcessorRuning() throws AdminException
  {
    Object result = callMethod(method_isTaskProcessorRuning_ID, Type.Types[Type.BooleanType], Collections.EMPTY_MAP);
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + '\"');
  }

  public void startTaskScheduler() throws AdminException
  {
    callMethod(method_startTaskScheduler_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public void stopTaskScheduler() throws AdminException
  {
    callMethod(method_stopTaskScheduler_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public boolean isTaskSchedulerRuning() throws AdminException
  {
    Object result = callMethod(method_isTaskSchedulerRuning_ID, Type.Types[Type.BooleanType], Collections.EMPTY_MAP);
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskSchedulerRuning: Incorrect return type \"" + result.getClass().getName() + '\"');
  }

  /**
   * add all tasks
   *
   * @param taskId - �����, ������� ���� �������� (��� ��� ������ ���� ��������� � �������)
   * @throws AdminException - ���� ���-�� �� ����������
   */
  public void addTask(String taskId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", taskId);
    callMethod(method_addTask_ID, Type.Types[Type.StringType], params);
  }

  /**
   * remove tasks
   *
   * @param taskId - �����, ������� ���� �������
   * @throws AdminException - ���� ���-�� �� ����������
   */
  public void removeTask(String taskId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", taskId);
    callMethod(method_removeTask_ID, Type.Types[Type.StringType], params);
  }

  public void changeTask(String taskId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", taskId);
    callMethod(method_changeTask_ID, Type.Types[Type.StringType], params);
  }

  public boolean isTaskEnabled(String taskId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", taskId);
    Object result = callMethod(method_isTaskEnabled_ID, Type.Types[Type.BooleanType], params);
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + '\"');
  }

  public void setTaskEnabled(String taskId, boolean enabled) throws AdminException
  {
    Map params = new HashMap(2);
    params.put("id", taskId);
    params.put("enabled", Boolean.valueOf(enabled));
    callMethod(method_setTaskEnabled_ID, Type.Types[Type.StringType], params);
  }

  public void startTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("ids", taskIds);
    callMethod(method_startTasks_ID, Type.Types[Type.StringType], params);
  }

  public void stopTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("ids", taskIds);
    callMethod(method_stopTasks_ID, Type.Types[Type.StringType], params);
  }

  public void addSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", scheduleId);
    callMethod(method_addSchedule_ID, Type.Types[Type.StringType], params);
  }

  public void removeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", scheduleId);
    callMethod(method_removeSchedule_ID, Type.Types[Type.StringType], params);
  }

  public void changeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap(1);
    params.put("id", scheduleId);
    callMethod(method_changeSchedule_ID, Type.Types[Type.StringType], params);
  }

  public void flushStatistics() throws AdminException
  {
    callMethod(method_flushStatistics_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public List getGeneratingTasks() throws AdminException
  {
    return (List) callMethod(method_getGeneratingTasksMethod_ID, Type.Types[Type.StringListType], Collections.EMPTY_MAP);
  }

  public List getProcessingTasks() throws AdminException
  {
    return (List) callMethod(method_getProcessingTasksMethod_ID, Type.Types[Type.StringListType], Collections.EMPTY_MAP);
  }


  public GetMessagesResult getMessages(final String taskId, final Message.State state, final Date fromSendDate, final Date toSendDate, final String address,
                          final String orderBy, final boolean orderAsc, final int limit) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");

    final Map args = new HashMap(8);
    args.put("id", taskId);
    args.put("state", (state == null || state == Message.State.UNDEFINED) ? "" : String.valueOf(state.getId()));
    args.put("from_date", (fromSendDate == null) ? "" : dateToString(fromSendDate));
    args.put("to_date", (toSendDate == null) ? "" : dateToString(toSendDate));
    args.put("order_by", (orderBy == null) ? "" : orderBy);
    args.put("order_direction", (orderAsc) ? "" : "desc");
    args.put("address", (address == null) ? "" : address);
    args.put("msg_limit", String.valueOf(limit));

    return translateGetMessagesResult((List)callMethod(SELECT_TASK_MESSAGES_METHOD_ID, Type.Types[Type.StringListType], args));
  }

  private static GetMessagesResult translateGetMessagesResult(List messages) throws AdminException {
    final List result = new LinkedList();
    int total = 0;

    try {
      String str;
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        str = (String) iter.next();

        if (iter.hasNext()) {
          final StringTokenizer st = new StringTokenizer(str, "|");
          final Message message = new Message();

          if (st.hasMoreTokens())
            message.setTaskId(st.nextToken());
          if (st.hasMoreTokens())
            message.setState(Message.State.getById(Integer.parseInt(st.nextToken())));
          if (st.hasMoreTokens())
            message.setAbonent(st.nextToken());
          if (st.hasMoreTokens())
            message.setSendDate(stringToDate(st.nextToken()));
          if (st.hasMoreTokens())
            message.setMessage(st.nextToken());

          result.add(message);

        } else
          total = Integer.parseInt(str);

      }

    } catch (Throwable e) {
      throw new AdminException(e.getMessage());
    }

    return new GetMessagesResult(result, total);
  }

  public void resendMessages(String taskId, String id, Message.State newState, Date newDate) throws AdminException {
    if (id == null)
      throw new AdminException("ID is null");

    resendMessages(taskId, null, null, id, null, null, newState, newDate);
  }

  public void resendMessages(String taskId, String abonent, Message.State state, Date fromDate, Date toDate,
                             Message.State newState, Date newDate) throws AdminException {

    resendMessages(taskId, fromDate, toDate, null, abonent, state, newState, newDate);
  }

  private void resendMessages(String taskId, Date fromDate, Date toDate, String id, String abonent, Message.State state,
                              Message.State newState, Date newDate) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");
    if (newState == null)
      throw new AdminException("New state is null");
    if (newDate == null)
      throw new AdminException("Send date is null");

    final Map args = new HashMap(8);
    args.put("id", taskId);
    args.put("date", dateToString(newDate));
    args.put("state", new Integer(newState.getId()));
    args.put("from_date", (fromDate == null) ? "" : dateToString(fromDate));
    args.put("record_id", (id == null) ? "" : id);
    args.put("address", (abonent == null) ? "" : abonent);
    args.put("to_date", (toDate == null) ? "" : dateToString(toDate));
    args.put("old_state", (state == null || state == Message.State.UNDEFINED) ? "" : String.valueOf(state.getId()));

    callMethod(CHANGE_DELIVERY_MESSAGE_INFO_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public void deleteMessages(String taskId, String id) throws AdminException {
    if (id == null)
      throw new AdminException("ID is null");

    deleteMessages(taskId, null, null, id, null, null);
  }

  public void deleteMessages(String taskId) throws AdminException {
    deleteMessages(taskId, null, null, null, null, null);
  }

  public void deleteMessages(String taskId, String abonent, Message.State state, Date fromDate, Date toDate) throws AdminException {
    deleteMessages(taskId, fromDate, toDate, null, abonent, state);
  }

  private void deleteMessages(String taskId, Date fromDate, Date toDate, String id, String abonent, Message.State state) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");

    final Map args = new HashMap(6);
    args.put("id", taskId);
    args.put("from_date", (fromDate == null) ? "" : dateToString(fromDate));
    args.put("record_id", (id == null) ? "" : id);
    args.put("address", (abonent == null) ? "" : abonent);
    args.put("to_date", (toDate == null) ? "" : dateToString(toDate));
    args.put("state", (state == null || state == Message.State.UNDEFINED) ? "" : String.valueOf(state.getId()));

    callMethod(DELETE_DELIVERY_MESSAGES_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public void changeDeliveryTextMessage(String taskId, String abonent, Message.State state, Date fromDate, Date toDate, String newTextMessage) throws AdminException {
    changeDeliveryTextMessage(taskId, fromDate, toDate, null, abonent, state, newTextMessage);
  }

  private void changeDeliveryTextMessage(String taskId, Date fromDate, Date toDate, String id, String abonent, Message.State state, String newTextMessage) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");
    if (newTextMessage == null)
      throw new AdminException("New text message is null");

    final Map args = new HashMap(7);
    args.put("id", taskId);
    args.put("from_date", (fromDate == null) ? "" : dateToString(fromDate));
    args.put("record_id", (id == null) ? "" : id);
    args.put("address", (abonent == null) ? "" : abonent);
    args.put("to_date", (toDate == null) ? "" : dateToString(toDate));
    args.put("state", (state == null || state == Message.State.UNDEFINED) ? "" : String.valueOf(state.getId()));
    args.put("new_text_message", newTextMessage);

    callMethod(CHANGE_DELIVERY_TEXT_MESSAGE_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public List getTaskStatistics(String taskId, Date fromDate, Date toDate) throws AdminException {

    final Map args = new HashMap(3);
    args.put("id", (taskId == null) ? "" : taskId);
    args.put("start_period", (fromDate == null) ? "" : calculatePeriod(fromDate));
    args.put("end_period", (toDate == null) ? "" : calculatePeriod(toDate));

    return translateGetTaskStatisticsResult((List)callMethod(SELECT_TASKS_STATISTIC_METHOD_ID, Type.Types[Type.StringListType], args));
  }

  private static List translateGetTaskStatisticsResult(List result) throws AdminException {
    final List res = new ArrayList(100);

    try {
      for (Iterator iter = result.iterator(); iter.hasNext();) {
        StringTokenizer st = new StringTokenizer((String)iter.next(), "|");
        final Statistic stat = new Statistic();

        if (st.hasMoreTokens())
          stat.setPeriod(calculatePeriod(st.nextToken()));
        if (st.hasMoreTokens())
          stat.setGenerated(Long.parseLong(st.nextToken()));
        if (st.hasMoreTokens())
          stat.setDelivered(Long.parseLong(st.nextToken()));
        if (st.hasMoreTokens())
          stat.setRetried(Long.parseLong(st.nextToken()));
        if (st.hasMoreTokens())
          stat.setFailed(Long.parseLong(st.nextToken()));

        res.add(stat);
      }
    } catch (Throwable e) {
      throw new AdminException(e.getMessage());
    }

    return res;
  }

  private static String calculatePeriod(Date date) {
    synchronized(sdf) {
      return sdf.format(date);
    }
  }

  private static Date calculatePeriod(String period) {
    Date converted;
    try {
      synchronized (sdf) {
        converted = sdf.parse(period);
      }
    } catch (ParseException e) {
      e.printStackTrace();
      converted = new Date();
    }
    return converted;
  }

  public void addDeliveryMessages(String taskId, Collection messages) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");
    if (messages == null)
      throw new AdminException("Message s list is null");

    final List list = new ArrayList(messages.size());
    // boolean putData = true;
    for (Iterator iter = messages.iterator(); iter.hasNext();) {
      Message m = (Message)iter.next();
        // FIXME: temporary adding userData to test new functionality
        // if ( m.getUserData() == null && putData ) {
        //     m.setUserData(String.valueOf(System.currentTimeMillis()));
        // }
        // putData = !putData;
      list.add(messageToString(m));
    }

    final Map args = new HashMap(2);
    args.put("id", taskId);
    args.put("messages", list);

    callMethod(ADD_DELIVERY_MESSAGES_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public void applyRetryPolicies() throws AdminException {
    callMethod(method_applyRetryPolicies_ID, Type.Types[Type.StringType], Collections.EMPTY_MAP);
  }

  public void addStatisticRecord(String taskId, Date period, int generated, int delivered, int retried, int failed) throws AdminException {
    if (taskId == null)
      throw new AdminException("Task id is null");
    if (period == null)
      throw new AdminException("Period is null");

    final Map args = new HashMap(6);
    args.put("id", taskId);
    args.put("period", Integer.valueOf(calculatePeriod(period)));
    args.put("generated", new Integer(generated));
    args.put("delivered", new Integer(delivered));
    args.put("retried", new Integer(retried));
    args.put("failed", new Integer(failed));

    callMethod(ADD_STATISTIC_RECORD_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public long sendSms(String source, String destination, String message, boolean flash) throws AdminException {
    final Map args = new HashMap(6);
    args.put("source", source);
    args.put("destination", destination);
    args.put("message", message);
    args.put("flash", Boolean.valueOf(flash));

    return ((Long)callMethod(method_sendSms_ID, Type.Types[Type.IntType], args)).intValue();
  }

  private static String messageToString(Message m) {
    StringBuffer b = new StringBuffer(100);
    b.append(m.getState().getId()).append('|')
            .append(m.getAbonent()).append('|')
            .append(dateToString(m.getSendDate())).append('|');
    if (m.getUserData() != null ) {
        b.append(m.getUserData()).append('|');
    }
    b.append(m.getMessage());
    return b.toString();
  }

  private synchronized Object callMethod(String methodId, Type resultType, Map arguments) throws AdminException {
    return call(SME_COMPONENT_ID, methodId, resultType, arguments);
  }

  private static String dateToString(final Date date) {
    synchronized(mdf) {
      return mdf.format(date);
    }
  }

  private static Date stringToDate(final String str) throws ParseException {
    synchronized(mdf) {
      return mdf.parse(str);
    }
  }

  public void endDeliveryMessageGeneration(String taskId) throws AdminException {
    Map params = new HashMap(1);
    params.put("id", taskId);
    callMethod(END_DELIVERY_MESSAGE_GENERATION_ID, Type.Types[Type.StringType], params);
  }

  public static class GetMessagesResult {
    private final Collection messages;
    private final int totalCount;

    public GetMessagesResult(Collection messages, int totalCount) {
      this.messages = messages;
      this.totalCount = totalCount;
    }

    public Collection getMessages() {
      return messages;
    }

    public int getTotalCount() {
      return totalCount;
    }
  }
}
