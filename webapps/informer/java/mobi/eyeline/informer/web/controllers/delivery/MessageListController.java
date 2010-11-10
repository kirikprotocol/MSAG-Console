package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class MessageListController extends InformerController{

  private String msisdn;

  private String state;

  private Date fromDate;

  private Date tillDate;

  public static final String DELIVERY_PARAM = "delivery";

  private final static int MEMORY_LIMIT = 100;     //todo

  private final Configuration config;

  private final User u;

  private Delivery delivery;

  private Integer deliveryId;

  private boolean init;

  private List<String> selected;

  private String errorCode;

  public MessageListController() {
    config = getConfig();
    u = getConfig().getUser(getUserName());
    String s = getRequestParameter(DELIVERY_PARAM);
    if(s != null && s.length() > 0) {
      deliveryId = Integer.parseInt(s);
      try {
        delivery = config.getDelivery(u.getLogin(), u.getPassword(), deliveryId);
      } catch (AdminException e) {
        addError(e);
      }
    }
  }

  public String getErrorCode() {
    return errorCode;
  }

  public void setErrorCode(String errorCode) {
    this.errorCode = errorCode;
  }

  @SuppressWarnings({"unchecked"})
  public void setSelected(List selected) {
    if(selected != null) {
      this.selected = new ArrayList<String>((List<String>) selected);
    }
  }

  public List<SelectItem> getUniqueStates() {
    List<SelectItem> result = new LinkedList<SelectItem>();
    for(MessageState st : MessageState.values()) {
      result.add(new SelectItem(st.toString(), MessageStateConverter.getAsString(getLocale(), st)));
    }
    return result;
  }

  public void query() {
    init = true;
  }

  public void clear() {
    init = false;
    fromDate = null;
    tillDate = null;
    state = null;
    msisdn = null;
  }

  public Delivery getDelivery() {
    return delivery;
  }

  public void setDelivery(Delivery delivery) {
    this.delivery = delivery;
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

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  public String getState() {
    return state;
  }

  public void setState(String state) {
    this.state = state;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  private MessageFilter getFilter() throws AdminException {
    MessageFilter filter = new MessageFilter(delivery.getId(), delivery.getStartDate(), new Date());
    filter.setFields(new MessageFields[]{MessageFields.Date, MessageFields.State, MessageFields.Text, MessageFields.ErrorCode, MessageFields.Abonent});
    if(fromDate != null) {
      filter.setStartDate(fromDate);
    }
    if(tillDate != null) {
      filter.setEndDate(tillDate);
    }
    if(state != null && state.length() > 0) {
      filter.setStates(new MessageState[]{MessageState.valueOf(state)});
    }
    if(msisdn != null && msisdn.length() > 0) {
      filter.setMsisdnFilter(new String[]{msisdn});
    }
    return filter;
  }

  @Override
  protected void _download(final PrintWriter writer) throws IOException {
    try {
      final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      visit(new Visitor<MessageInfo>() {
        public boolean visit(MessageInfo value) throws AdminException {
          writer.println(StringEncoderDecoder.toCSVString(
              new String[]{value.getAbonent(), sdf.format(value.getDate()), value.getText(),
                  value.getState().toString(), value.getErrorCode() == null ? "" : value.getErrorCode().toString()}
          ));
          return true;
        }
      });
    }catch (AdminException e) {
      addError(e);
    }
  }

  private void visit(Visitor<MessageInfo> visitor) throws AdminException{
    MessageFilter filter = getFilter();
    config.getMessagesStates(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, visitor);
  }

  public DataTableModel getMessages() {

    return new DataTableModel() {
      public List getRows(final int startPos, final int count, final DataTableSortOrder sortOrder) {
        if(!init) {
          return Collections.emptyList();
        }
        final LinkedList<MessageInfo> list = new LinkedList<MessageInfo>();
        try{
          final int[] c = new int[]{0};
          visit(new Visitor<MessageInfo>() {
            public boolean visit(MessageInfo value) throws AdminException {
              c[0]++;
              if(c[0] > startPos) {
                list.add(value);
              }
              return list.size() < count;
            }
          });
        }catch (AdminException e){
          addError(e);
        }
        return list;
      }

      public int getRowsCount() {
        try {
          return config.countMessages(u.getLogin(), u.getPassword(), getFilter());
        } catch (AdminException e) {
          addError(e);
        }
        return 0;
      }
    };
  }

}
