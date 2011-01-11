package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.LocalizedException;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import javax.servlet.http.HttpSession;
import java.io.*;
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

  private Delivery.Type deliveryType;

  private Integer deliveryId;

  private boolean showTexts;

  public MessageListController() {
    config = getConfig();
    u = getConfig().getUser(getUserName());

    String s = getRequestParameter(DELIVERY_PARAM);

    HttpSession session = getSession(false);
    thread = (ResendThread) session.getAttribute(THREAD_NAME);

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

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  public int getResendCurrent() {
    return thread != null ? thread.resendCurrent : 0;
  }

  public int getResendTotal() {
    return thread != null ? thread.resendTotal : Integer.MAX_VALUE;
  }

  private void loadDeliveryOptions() {
    if (deliveryId != null) {
      try {
        Delivery delivery = config.getDelivery(u.getLogin(), u.getPassword(), deliveryId);
        msgFilter.setFromDate(delivery.getStartDate());
        msgFilter.setTillDate(delivery.getEndDate() == null ? new Date(System.currentTimeMillis() + (24 * 60 * 60 * 1000)) : delivery.getEndDate());
        deliveryName = delivery.getName();
        deliveryType = delivery.getType();
      } catch (AdminException e) {
        addError(e);
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

  public String stop() {
    if (thread != null) {
      thread.stop = true;
    }
    return null;
  }


  public MsgFilter getMsgFilter() {
    return msgFilter;
  }

  public int getState() {
    return state;
  }

  public String getError() {
    return thread != null && thread.finished ? thread.error : null;
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

  private static final int MEMORY_LIMIT = 1000;

  public String resendAll() {
    try {
      final MessageFilter filter = getModelFilter();
      HttpSession session = getSession(false);
      if (session != null) {
        Thread thread = new ResendThread(deliveryType == Delivery.Type.SingleText, filter.getDeliveryId(), u, config, filter, null, getLocale());
        thread.start();
        session.setAttribute(THREAD_NAME, thread);
        state = 1;
      }
    } catch (LocalizedException e) {
      addError(e);
    }
    return null;
  }

  public String resend() {
    if (selected == null || selected.isEmpty()) {
      return null;
    }
    try {
      final MessageFilter filter = getModelFilter();
      HttpSession session = getSession(false);
      if (session != null) {
        ResendThread thread = new ResendThread(deliveryType == Delivery.Type.SingleText, filter.getDeliveryId(), u, config, filter, new ArrayList<String>(selected), getLocale());
        thread.start();
        session.setAttribute(THREAD_NAME, thread);
        state = 1;
      }
    } catch (LocalizedException e) {
      addError(e);
    }
    return null;
  }

  private MessageFilter getModelFilter() throws LocalizedException {
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
    return filter;
  }

  @Override
  protected void _download(final PrintWriter writer) throws IOException {
    try {
      final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      visit(new Visitor<Message>() {
        public boolean visit(Message value) throws AdminException {
          writer.println(StringEncoderDecoder.toCSVString(value.getAbonent().getSimpleAddress(), sdf.format(value.getDate()), value.getText(),
                  value.getState().toString(), value.getErrorCode() == null ? "" : value.getErrorCode().toString()
          ));
          return true;
        }
      }, getModelFilter());
    } catch (LocalizedException e) {
      addError(e);
    }
  }

  private void visit(Visitor<Message> visitor, MessageFilter filter) throws AdminException {
    config.getMessagesStates(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, visitor);
  }

  public DataTableModel getMessages() {
    if (state == 1) {
      return new EmptyDataTableModel();
    }

    final MessageFilter filter;
    try {
      filter = getModelFilter();
    } catch (LocalizedException e) {
      addError(e);
      return new EmptyDataTableModel();
    }
    return new DataTableModel() {
      public List getRows(final int startPos, final int count, final DataTableSortOrder sortOrder) {
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
          addError(e);
        }
        return list;
      }

      public int getRowsCount() {
        try {
          return config.countMessages(u.getLogin(), u.getPassword(), getModelFilter());
        } catch (LocalizedException e) {
          addError(e);
        }
        return 0;
      }
    };
  }

  public boolean isShowTexts() {
    return showTexts;
  }

  public void setShowTexts(boolean showTexts) {
    this.showTexts = showTexts;
  }

  public static class ResendThread extends Thread {

    private String error;

    private User u;

    private Configuration config;

    private MessageFilter filter;

    private List<String> selected;

    private Locale locale;

    private boolean finished;

    private int resended = 0;

    private int deliveryId;

    private boolean isSingleText;

    private boolean stop;

    private int resendCurrent;

    private int resendTotal = Integer.MAX_VALUE;

    public ResendThread(boolean isSingleText, int deliveryId, User u, Configuration config, MessageFilter filter, List<String> selected, Locale locale) {
      this.deliveryId = deliveryId;
      this.u = u;
      this.config = config;
      this.filter = filter;
      this.selected = selected;
      this.locale = locale;
      this.isSingleText = isSingleText;
    }

    private int getAndSave(File workFile, final Set<Long> sld) throws AdminException {
      final PrintWriter[] w = new PrintWriter[]{null};

      try {
        w[0] = new PrintWriter(new BufferedWriter(new OutputStreamWriter(config.getFileSystem().getOutputStream(workFile, false))));

        final int[] count = new int[]{0};

        config.getMessagesStates(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, new Visitor<Message>() {
          public boolean visit(Message value) throws AdminException {
            if (stop) {
              return false;
            }
            if (sld != null && !sld.remove(value.getId())) {
              return true;
            }
            switch (value.getState()) {
              case Process:
              case New:
                break;
              default:
                if (!isSingleText) {
                  w[0].print(value.getAbonent());
                  w[0].print(',');
                  w[0].println(value.getText());
                } else {
                  w[0].println(value.getAbonent());
                }
                count[0]++;
            }
            return sld == null || !sld.isEmpty();
          }
        });
        resendTotal = 2 * count[0];
        return resendCurrent = count[0];
      } finally {
        if (w[0] != null) {
          w[0].close();
        }
      }
    }

    private void readAndProccess(File workFile) throws AdminException {
      final BufferedReader[] r = new BufferedReader[]{null};
      try {
        r[0] = new BufferedReader(new InputStreamReader(config.getFileSystem().getInputStream(workFile)));
        if (!isSingleText) {
          config.addMessages(u.getLogin(), u.getPassword(), new DataSource<Message>() {
            public Message next() throws AdminException {
              if (stop) {
                return null;
              }
              try {
                String line = r[0].readLine();
                if (line == null) {
                  return null;
                }
                String[] s = line.split(",", 2);
                resended++;
                resendCurrent++;
                return Message.newMessage(new Address(s[0]), s[1]);
              } catch (IOException e) {
                e.printStackTrace();
                return null;
              }
            }
          }, filter.getDeliveryId());
        } else {
          config.addSingleTextMessages(u.getLogin(), u.getPassword(), new DataSource<Address>() {
            public Address next() throws AdminException {
              if (stop) {
                return null;
              }
              try {
                String line = r[0].readLine();
                if (line == null) {
                  return null;
                }
                resended++;
                resendCurrent++;
                return new Address(line);
              } catch (IOException e) {
                e.printStackTrace();
                return null;
              }
            }
          }, filter.getDeliveryId());
        }
      } finally {
        if (r[0] != null) {
          try {
            r[0].close();
          } catch (Exception ignored) {
          }
        }
      }
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

        File workFile = new File(config.getWorkDir(), "resend_" + u.getLogin() + System.currentTimeMillis());

        try {
          if (!stop) {
            if (getAndSave(workFile, sld) != 0 && !stop) {
              readAndProccess(workFile);
            }
          }
          if (!stop) {
            resendCurrent = resendTotal;
            config.activateDelivery(u.getLogin(), u.getPassword(), filter.getDeliveryId());
          }
        } finally {
          try {
            config.getFileSystem().delete(workFile);
          } catch (Exception ignored) {
          }
        }

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

  }

}
