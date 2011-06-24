package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.ArchiveMessage;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.Identificator;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.controllers.InformerController;
import mobi.eyeline.informer.web.controllers.users.UserEditController;

import javax.faces.application.FacesMessage;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesResultController extends InformerController{

  private Date from;

  private Date till;

  private Address address;

  private Integer reqId;

  private List selected;

  public MessagesResultController() {
    String id = getRequestParameter("reqId");
    if(id == null || id.length() == 0) {
      id = getRequestParameter(UserEditController.COME_BACK_PARAMS);
    }
    if(id != null && id.length()>0) {
      reqId = Integer.parseInt(id);
    }else {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", "");
    }
  }


  public void clearFilter() {
    address = null;
    from = null;
    till = null;
  }

  public void query() {
  }

  /**
   * Вставляет объект info в отсортированный массив infos.
   *
   * @param infos отсортированный массив
   * @param info  объект для вставки
   * @param c     компаратор, при помощи которого отсортирован массив
   * @param <T>   тип элементов массива
   */
  private static <T> void insert(T[] infos, T info, Comparator<T> c) {
    int sRes = Arrays.binarySearch(infos, info, c);

    if (sRes < 0)
      sRes = -sRes - 1;

    if (infos[sRes] != null)
      System.arraycopy(infos, sRes, infos, sRes + 1, infos.length - sRes - 1);

    infos[sRes] = info;
  }

  @SuppressWarnings({"RedundantIfStatement"})
  private boolean accept(ArchiveMessage value) {
    if(address != null && !value.getAbonent().getSimpleAddress().startsWith(address.getSimpleAddress())) {
      return false;
    }
    if(from != null && value.getDate() != null && value.getDate().before(from)) {
      return false;
    }
    if(till != null && value.getDate() != null &&value.getDate().after(till)) {
      return false;
    }
    return true;
  }

  private List<ArchiveMessage> getMessages(final int startPos, final int count, DataTableSortOrder sortOrder) throws AdminException {
    final Comparator<ArchiveMessage> comparator = getComparator(sortOrder);

    final ArchiveMessage infos[] = new ArchiveMessage[startPos + count];
    final int lastIdx = infos.length - 1;

    getConfig().getMessagesResult(reqId, new Visitor<ArchiveMessage>() {
      public boolean visit(ArchiveMessage value) throws AdminException {
        if(!accept(value)) {
          return true;
        }
        if (infos[lastIdx] == null || comparator.compare(value, infos[lastIdx]) < 0)
          insert(infos, value, comparator);
        return true;
      }
    });

    return Arrays.asList(infos).subList(startPos, startPos + count);
  }


  public DataTableModel getMessages() {
    if(reqId == null) {
      return new EmptyDataTableModel();
    }

    List<User> users = getConfig().getUsers();
    final Set<String>existUsers = new HashSet<String>(users.size());
    for(User u : users) {
      existUsers.add(u.getLogin());
    }

    class DataTableModelImpl implements DataTableModel, Identificator {
      public List getRows(final int startPos, final int count, DataTableSortOrder sortOrder) {
        try {
          List<ArchiveMessage> messages = getMessages(startPos, count, sortOrder);
          List<MessageRow> rows = new ArrayList<MessageRow>(messages.size());
          for(ArchiveMessage d : messages) {
            if(d != null) {
              rows.add(new MessageRow(d, existUsers.contains(d.getOwner())));
            }
          }
          return rows;
        } catch (AdminException e) {
          addError(e);
          return Collections.emptyList();
        }
      }

      @Override
      public String getId(Object value) {
        return Long.toString(((MessageRow)value).getId());
      }

      public int getRowsCount() {
        try {
          final int[] count = new int[]{0};
          getConfig().getMessagesResult(reqId, new Visitor<ArchiveMessage>() {
            @Override
            public boolean visit(ArchiveMessage value) throws AdminException {
              if(accept(value)) {
                count[0]++;
              }
              return true;
            }
          });
          return count[0];
        } catch (AdminException e) {
          addError(e);
          return 0;
        }
      }
    };
    return new DataTableModelImpl();
  }

  public static class MessageRow {

    private ArchiveMessage message;

    private boolean userExist;

    public MessageRow(ArchiveMessage message, boolean userExist) {
      this.message = message;
      this.userExist = userExist;
    }

    public long getId() {
      return message.getId();
    }

    public Address getAbonent() {
      return message.getAbonent();
    }
    public String getOwner() {
      return message.getOwner();
    }

    public Date getDate() {
      return message.getDate();
    }

    public boolean isUserExist() {
      return userExist;
    }
    public String getText() {
      return message.getText();
    }
    public MessageState getState() {
      return message.getState();
    }

    public Integer getErrorCode() {
      return message.getErrorCode();
    }

    public int getDeliveryId() {
      return message.getDeliveryId();
    }

    public String getDeliveryName() {
      return message.getDeliveryName();
    }
  }

  private static Comparator<ArchiveMessage> getComparator(final DataTableSortOrder sortOrder) {
    if (sortOrder == null || sortOrder.getColumnId().equals("abonent")) {
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getAbonent().getSimpleAddress().compareTo(o2.getAbonent().getSimpleAddress()) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("userId")) {
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getOwner().compareTo(o2.getOwner()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("date")) {
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getDate().compareTo(o2.getDate()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("deliveryName")) {
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getDeliveryName().compareTo(o2.getDeliveryName()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("state")){
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          if (o1.getState() == null) {
            return o2.getState() == null ? 0 : (sortOrder.isAsc() ? -1 : 1);
          } else if (o2.getState() == null) {
            return (sortOrder.isAsc() ? 1 : -1);
          }
          return o1.getState().compareTo(o2.getState()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    }else {
      return new Comparator<ArchiveMessage>() {
        public int compare(ArchiveMessage o1, ArchiveMessage o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          if (o1.getErrorCode() == null) {
            return o2.getErrorCode() == null ? 0 : (sortOrder.isAsc() ? -1 : 1);
          } else if (o2.getErrorCode() == null) {
            return (sortOrder.isAsc() ? 1 : -1);
          }
          return o1.getErrorCode().compareTo(o2.getErrorCode()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    }
  }


  public List getSelected() {
    return selected;
  }

  public void setSelected(List selected) {
    this.selected = selected;
  }

  public String getSelectedAsString() {
    if (selected == null || selected.isEmpty())
      return "";

    StringBuilder sb = new StringBuilder();
    for (Object s : selected) {
      if (sb.length() > 0)
        sb.append(',');
      sb.append(s);
    }

    return sb.toString();
  }

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }

  public Integer getReqId() {
    return reqId;
  }

  public void setReqId(Integer reqId) {
    this.reqId = reqId;
  }

  @Override
  protected void _download(final PrintWriter writer) throws IOException {
    if(reqId != null) {
      try {
        final SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy.dd.MM HH:mm");
        getConfig().getMessagesResult(reqId, new Visitor<ArchiveMessage>() {
          @Override
          public boolean visit(ArchiveMessage value) throws AdminException {
            writer.println(StringEncoderDecoder.toCSVString(';', value.getAbonent().getSimpleAddress(),
                value.getDeliveryName(), value.getOwner(), value.getText(), value.getDate() != null ? dateFormat.format(value.getDate()) : " ", value.getState(),
                value.getErrorCode() != null ? value.getErrorCode() : " "));
            return true;
          }
        });
      } catch (AdminException e) {
        addError(e);
      }
    }
    super._download(writer);
  }
}
