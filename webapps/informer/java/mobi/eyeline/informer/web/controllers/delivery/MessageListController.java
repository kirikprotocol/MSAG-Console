package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.LocalizedException;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import javax.servlet.http.HttpSession;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class MessageListController extends InformerController {

  private MsgFilter msgFilter = new MsgFilter();

  public static final String DELIVERY_PARAM = "delivery";

  private final Configuration config;

  private final User u;

  private boolean init;

  private List<String> selected;

  private int state = 0;

  private static final String THREAD_NAME = "resend_thread";

  private ResendThread thread;

  private String deliveryName;

  private Integer deliveryId;

  private boolean showTexts;

  private boolean archive;

  private DeliveryStrategy strategy;

  private String error;

  private String comeBackAction;

  private String comeBackParams;

  private boolean noResended;

  public static final String COME_BACK = "MESSAGES_COME_BACK" ;
  public static final String COME_BACK_PARAMS = "MESSAGES_COME_BACK_PARAMS";

  public MessageListController() {
    config = getConfig();

    archive = Boolean.valueOf(getRequestParameter("archive"));
    strategy = getStrategy();

    u = getConfig().getUser(getUserName());

    String s = getRequestParameter(DELIVERY_PARAM);

    HttpSession session = getSession(false);
    thread = (ResendThread) session.getAttribute(THREAD_NAME);

    comeBackParams = getRequestParameter(COME_BACK_PARAMS);
    comeBackAction = getRequestParameter(COME_BACK);

    if (s != null && s.length() > 0) {
      deliveryId = Integer.parseInt(s);
    } else {
      if (thread != null) {
        deliveryId = thread.deliveryId;
      }
    }
    if (thread != null) {
      if (deliveryId != null && thread.deliveryId != deliveryId) {
        session.removeAttribute(THREAD_NAME);
      } else {
        state = 1;
      }
    }
    if (state == 0) {
      loadDeliveryOptions();
    }
  }

  public boolean isNoResended() {
    return noResended;
  }

  public void setNoResended(boolean noResended) {
    this.noResended = noResended;
  }

  public String getComeBackName() {
    return COME_BACK_PARAMS;
  }

  public String getComeBackAction() {
    return comeBackAction;
  }


  public String comeBackAction() {
    return comeBackAction;
  }

  public void setComeBackAction(String comeBackAction) {
    this.comeBackAction = comeBackAction;
  }

  public String getComeBackParams() {
    return comeBackParams;
  }

  public void setComeBackParams(String comeBackParams) {
    this.comeBackParams = comeBackParams;
  }

  public boolean isResendAllowed() {
    return strategy.isChangesAllowed();
  }

  private DeliveryStrategy getStrategy() {
    return archive ? new ArchiveDeliveryStrategy(config) : new CommonDeliveryStrategy(config);
  }

  public boolean isArchive() {
    return archive;
  }

  public void setArchive(boolean archive) {
    this.archive = archive;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  public int getResendCurrent() {
    if(thread == null) {
      return 0;
    }else if(thread.finished && thread.resendTotal == 0) {
      return Integer.MAX_VALUE;
    }else {
      return thread.resended;
    }
  }

  public int getResendTotal() {
    return thread == null || thread.resendTotal == 0 ? Integer.MAX_VALUE : thread.resendTotal;
  }

  private void loadDeliveryOptions() {
    if (deliveryId != null) {
      try {
        Delivery delivery = strategy.getDelivery(u.getLogin(), deliveryId);
        msgFilter.setFromDate(new Date(delivery.getCreateDate().getTime()));
        msgFilter.setTillDate(
            new Date((delivery.getEndDate() == null ?
                          System.currentTimeMillis() :
                          delivery.getEndDate().getTime())
                     + (24 * 60 * 60 * 1000))
        );
        deliveryName = delivery.getName();
      } catch (AdminException e) {
        logger.error(e, e);
        error = e.getMessage(getLocale());
        msgFilter.setFromDate(new Date());
        msgFilter.setTillDate(new Date(System.currentTimeMillis() + (24 * 60 * 60 * 1000)));
      }
    }
  }

  public String getDeliveryName() {
    return deliveryName;
  }

  public void setDeliveryName(String deliveryName) {
    this.deliveryName = deliveryName;
  }

  public String resendFinish() {
    HttpSession session = getSession(false);
    if (session != null) {
      session.removeAttribute(THREAD_NAME);
      state = 0;
      loadDeliveryOptions();
    }
    return null;
  }

  public MsgFilter getMsgFilter() {
    return msgFilter;
  }

  public int getState() {
    return state;
  }

  public String getResendError() {
    return thread != null && thread.finished ? thread.error : null;
  }

  public String getError() {
    return error;
  }

  public boolean isFinished() {
    return thread != null && thread.finished;
  }

  public int getResended() {
    return thread != null ? thread.resended : 0;
  }

  @SuppressWarnings({"unchecked"})
  public void setSelected(List selected) {
    if (selected != null) {
      this.selected = new ArrayList<String>((List<String>) selected);
    }
  }

  public List<SelectItem> getUniqueStates() {
    List<SelectItem> result = new LinkedList<SelectItem>();
    for (MsgState st : MsgState.values()) {
      result.add(new SelectItem(st, MsgStateConverter.getAsString(getLocale(), st)));
    }
    return result;
  }

  public void query() {
    init = true;
  }

  public void clear() {
    init = false;
    noResended = false;
    msgFilter.setErrorCode(null);
    msgFilter.setMsisdn(null);
    msgFilter.setState(null);
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public String getDeliveryParam() {
    return DELIVERY_PARAM;
  }

  private static final int MEMORY_LIMIT = 10000;

  public String resendAll() {
    final MessageFilter filter = getModelFilter();
    HttpSession session = getSession(false);
    if (session != null) {
      Thread thread = new ResendThread(filter.getDeliveryId(), u, config, filter, null, getLocale());
      thread.start();
      session.setAttribute(THREAD_NAME, thread);
      state = 1;
    }
    return null;
  }

  public String resend() {
    if (selected == null || selected.isEmpty()) {
      return null;
    }
    final MessageFilter filter = getModelFilter();
    HttpSession session = getSession(false);
    if (session != null) {
      ResendThread thread = new ResendThread(filter.getDeliveryId(), u, config, filter, new ArrayList<String>(selected), getLocale());
      thread.start();
      session.setAttribute(THREAD_NAME, thread);
      state = 1;
    }
    return null;
  }

  private MessageFilter getModelFilter() {
    MessageFilter filter = new MessageFilter(deliveryId, new Date(msgFilter.getFromDate().getTime()), new Date(msgFilter.getTillDate().getTime()));
    if (msgFilter.getState() != null ) {
      filter.setStates(msgFilter.getState().toMessageStates());
    }
    if (msgFilter.getMsisdn() != null) {
      filter.setMsisdnFilter(msgFilter.getMsisdn().getSimpleAddress());
    }
    if (msgFilter.getErrorCode() != null) {
      filter.setErrorCodes(msgFilter.getErrorCode());
    }
    filter.setNoResended(noResended);
    return filter;
  }

  @Override
  protected void _download(final PrintWriter writer) throws IOException {
    try {
      final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      writer.print("Abonent;Date;");
      if(showTexts) {
        writer.print("Text;");
      }
      writer.println("State;Resended;ErrorCode;");
      visit(new Visitor<Message>() {
        public boolean visit(Message value) throws AdminException {
          writer.print(StringEncoderDecoder.toCSVString(';', value.getAbonent().getSimpleAddress(), sdf.format(value.getDate())));
          if(showTexts) {
            writer.print(';');
            writer.print(StringEncoderDecoder.toCSVString(';', value.getText()));
          }
          writer.print(';');
          writer.println(
              StringEncoderDecoder.toCSVString(';', value.getState().toString(), Boolean.valueOf(value.isResended()), value.getErrorCode() == null ? "" : value.getErrorCode().toString()
              ));
          return true;
        }
      }, getModelFilter());
    } catch (LocalizedException e) {
      addError(e);
    }
  }

  private void visit(Visitor<Message> visitor, MessageFilter filter) throws AdminException {
    strategy.getMessagesStates(u.getLogin(), filter, visitor);
  }

  public DataTableModel getMessages() {
    if (state == 1) {
      return new EmptyDataTableModel();
    }

    final MessageFilter filter = getModelFilter();

    return new ModelWithObjectIds() {
      public List getRows(final int startPos, final int count, final DataTableSortOrder sortOrder) throws ModelException{
        if (!init || state == 1) {
          return Collections.emptyList();
        }
        final LinkedList<Message> list = new LinkedList<Message>();
        try {
          final int[] c = new int[]{0};
          visit(new Visitor<Message>() {
            public boolean visit(Message value) throws AdminException {
              c[0]++;
              if (c[0] > startPos) {
                list.add(value);
              }
              return list.size() < count;
            }
          }, filter);
        } catch (AdminException e) {
          throw new ModelException(e.getMessage(getLocale()));
        }
        return list;
      }

      @Override
      public String getId(Object value) {
        return ((Message)value).getId().toString();
      }

      public int getRowsCount() throws ModelException{
        try {
          return strategy.countMessages(u.getLogin(), filter);
        } catch (LocalizedException e) {
          logger.error(e, e);
          throw new ModelException(e.getMessage(getLocale()));
        }
      }
    };
  }

  public boolean isShowTexts() {
    return showTexts;
  }

  public void setShowTexts(boolean showTexts) {
    this.showTexts = showTexts;
  }

  public class ResendThread extends Thread {

    private String error;

    private User u;

    private Configuration config;

    private List<String> selected;

    private Locale locale;

    private boolean finished;

    private int resended = 0;

    private int deliveryId;

    private int resendTotal = 0;

    private final MessageFilter filter;

    public ResendThread(int deliveryId, User u, Configuration config, MessageFilter filter, List<String> selected, Locale locale) {
      this.deliveryId = deliveryId;
      this.u = u;
      this.config = config;
      this.selected = selected;
      this.locale = locale;
      this.filter = filter;
    }

    public void run() {
      try {
        final Set<Long> sld;
        if (selected != null) {
          sld = new HashSet<Long>(selected.size());
          for (String s : selected) {
            sld.add(Long.parseLong(s));
          }
        } else {
          sld = null;
        }

        if(sld != null) {
          config.resend(u.getLogin(), deliveryId, sld, new ResendListenerImpl());
        }else {
          config.resendAll(u.getLogin(), deliveryId, filter, new ResendListenerImpl());
        }

        resended = resendTotal;

      } catch (AdminException e) {
        error = e.getMessage(locale);
      } catch (Exception e) {
        logger.error(e, e);
        ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
        error = bundle.getString("internal.error");
      } finally {
        finished = true;
      }
    }

    private class ResendListenerImpl implements ResendListener {
      @Override
      public void resended(long messageId, int totalSize) {
        resended++;
        resendTotal = totalSize;
      }
    }

  }

  private static interface DeliveryStrategy {

    boolean isChangesAllowed();

    Delivery getDelivery(String login, int deliveryId) throws AdminException;

    void getMessagesStates(String login, MessageFilter filter, Visitor<Message> visitor) throws AdminException;

    int countMessages(String login, MessageFilter filter) throws AdminException;

    void activateDelivery(String login, int deliveryId) throws AdminException;

  }

  private class CommonDeliveryStrategy implements DeliveryStrategy {

    private Configuration config;

    private CommonDeliveryStrategy(Configuration config) {
      this.config = config;
    }

    @Override
    public boolean isChangesAllowed() {
      return true;
    }

    @Override
    public Delivery getDelivery(String login, int deliveryId) throws AdminException {
      return config.getDelivery(login, deliveryId);
    }

    @Override
    public void getMessagesStates(String login, MessageFilter filter, Visitor<Message> visitor) throws AdminException {
      config.getMessagesStates(login, filter, MEMORY_LIMIT, visitor);
    }

    @Override
    public int countMessages(String login, MessageFilter messageFilter) throws AdminException {
      return config.countMessages(login, messageFilter);
    }

    @Override
    public void activateDelivery(String login, int deliveryId) throws AdminException {
      config.activateDelivery(login, deliveryId);
    }
  }

  private class ArchiveDeliveryStrategy implements DeliveryStrategy {

    private Configuration config;

    private ArchiveDeliveryStrategy(Configuration config) {
      this.config = config;
    }

    @Override
    public boolean isChangesAllowed(){
      return false;
    }

    @Override
    public Delivery getDelivery(String login, int deliveryId) throws AdminException {
      return config.getArchiveDelivery(login, deliveryId);
    }

    @Override
    public void getMessagesStates(String login, MessageFilter filter, Visitor<Message> visitor) throws AdminException {
      config.getArchiveMessages(login, filter, MEMORY_LIMIT, visitor);
    }

    @Override
    public int countMessages(String login, MessageFilter filter) throws AdminException {
      return config.countArchiveMessages(login, filter);
    }

    @Override
    public void activateDelivery(String login, int deliveryId) throws AdminException {
      throw new IllegalAccessError();
    }
  }
}
