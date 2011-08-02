package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByRecController extends InformerController {
  private User user;
  private boolean initError = false;
  private List<MessagesByRecRecord> records;


  private Date fromDate;
  private Date tillDate;
  private Address msisdn;
  private String allowedUser;

  private boolean loaded;

  private boolean init;

  private LoadListener loadListener;

  public MessagesByRecController() {
    super();
    records = Collections.synchronizedList(new ArrayList<MessagesByRecRecord>());
    initUser();
  }

  private void initUser() {
    user = getConfig().getUser(getUserName());
    if (user == null) initError = true;
    else if (!user.hasRole(User.INFORMER_ADMIN_ROLE)) {
      allowedUser = user.getLogin();
    } else {
      allowedUser = null;
    }
  }

  public void clearFilter() {
    loaded = false;
    loadListener = null;
    initUser();
    setFromDate(null);
    setTillDate(null);
  }

  public String start() {
    loaded = false;
    loadListener = null;
    init = true;
    return null;
  }

  public boolean isInit() {
    return init;
  }

  public boolean isLoaded() {
    return loaded;
  }

  public void setLoaded(boolean loaded) {
    this.loaded = loaded;
  }

  public void setUser(User user) {
    this.user = user;
  }

  public User getUser() {
    return user;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }


  public boolean isAdmin() {
    return user.hasRole(User.INFORMER_ADMIN_ROLE);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem("",""));
    for (User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin(),u.getLogin()));
    }
    Collections.sort(ret, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ret;
  }

  public void execute(final Configuration config, final Locale locale, final LoadListener listener) throws AdminException {
    records.clear();

    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setCreateDateFrom(fromDate);
    deliveryFilter.setEndDateTo(tillDate);
    if (allowedUser != null) {
      deliveryFilter.setUserIdFilter(allowedUser);
    }

    listener.setCurrent(0);
    listener.setTotal(config.countDeliveries(getUser().getLogin(), deliveryFilter));
    config.getDeliveries(getUser().getLogin(), deliveryFilter, 1000,
        new DeliveryVisitorImpl(config, locale, listener)
    );
  }


  public synchronized DataTableModel getRecords() {

    final Locale locale = getLocale();

    final Configuration config = getConfig();

    return new PreloadableModel() {

      @Override
      public LoadListener prepareRows(int startPos, int count, DataTableSortOrder sortOrder) {
        LoadListener listener = null;
        if(!loaded) {
          if(loadListener == null) {
            loadListener = new LoadListener();
            new Thread() {
              public void run() {
                try{
                  MessagesByRecController.this.execute(config, locale, loadListener);
                  loaded = true;
                }catch (AdminException e){
                  logger.error(e,e);
                  loadListener.setLoadError(new ModelException(e.getMessage(locale)));
                }catch (Exception e){
                  logger.error(e,e);
                }
              }
            }.start();
          }
          listener = loadListener;
        }
        return listener;
      }

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, new Comparator<MessagesByRecRecord>() {

            public int compare(MessagesByRecRecord o1, MessagesByRecRecord o2) {

              final int mul = sortOrder.isAsc() ? 1 : -1;
              if (sortOrder.getColumnId().equals("name")) {
                return mul * o1.getName().compareTo(o2.getName());
              }
              if (sortOrder.getColumnId().equals("userId")) {
                return mul * o1.getUserId().compareTo(o2.getUserId());
              }
              if (sortOrder.getColumnId().equals("text")) {
                if (o1.getText() == null && o1.getText() == null) return 0;
                if (o1.getText() == null) return mul;
                return mul * o1.getText().compareTo(o2.getText());
              }
              if (sortOrder.getColumnId().equals("deliveryDate")) {
                return mul * o1.getDeliveryDate().compareTo(o2.getDeliveryDate());
              }
              if (sortOrder.getColumnId().equals("state")) {
                return mul * o1.getState().compareTo(o2.getState());
              }
              if (sortOrder.getColumnId().equals("errorString")) {
                return mul * o1.getErrorString().compareTo(o2.getErrorString());
              }
              return 0;
            }
          });
        }

        List<MessagesByRecRecord> result = new LinkedList<MessagesByRecRecord>();
        for (Iterator<MessagesByRecRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          MessagesByRecRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return records.size();
      }
    };
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    for (MessagesByRecRecord r : records) {
      r.printCSV(writer);
    }
  }


  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public Address getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(Address msisdn) {
    this.msisdn = msisdn;
  }

  //

  private class DeliveryVisitorImpl implements Visitor<Delivery> {
    private final Configuration config;
    private final Locale locale;
    private final LoadListener listener;

    public DeliveryVisitorImpl(Configuration config, Locale locale, LoadListener listener) {
      this.config = config;
      this.locale = locale;
      this.listener = listener;
    }

    public boolean visit(Delivery delivery) throws AdminException {
      final int deliveryId = delivery.getId();
      final String name = delivery.getName();
      final String userId = delivery.getOwner();
      final User owner = config.getUser(userId);

      MessageFilter messageFilter = new MessageFilter(deliveryId, fromDate != null ? fromDate : delivery.getCreateDate(),
          tillDate != null ? tillDate : delivery.getEndDate() != null ? delivery.getEndDate() : new Date());
      messageFilter.setMsisdnFilter(msisdn.getSimpleAddress());

      config.getMessagesStates(getUser().getLogin(), messageFilter, 1000,
          new Visitor<Message>() {
            public boolean visit(Message message) throws AdminException {
              String errString = null;
              Integer errCode = message.getErrorCode();
              if (errCode != null) {
                try {
                  errString = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", locale).getString("informer.errcode." + errCode);
                }
                catch (MissingResourceException ex) {
                  errString = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", locale).getString("informer.errcode.unknown");
                }
              }
              MessagesByRecRecord rec = new MessagesByRecRecord(message.getAbonent().getSimpleAddress(), deliveryId, userId, owner, name, message.getText(), message.getDate(), message.getState(), errString);
              records.add(rec);
              return true;
            }
          }
      );
      listener.setCurrent(listener.getCurrent() + 1);
      return true;
    }
  }
}