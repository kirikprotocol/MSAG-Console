package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;

import java.io.Serializable;
import java.nio.charset.Charset;
import java.util.*;
import java.util.regex.Pattern;

/**
 * Настройки пользователя
 *
 * @author Aleksandr Khalitov
 */
public class User implements Serializable {

  private String login;
  private String password;
  private Status status;
  private String firstName;
  private String lastName;
  private Address phone;
  private String email;
  private String organization;
  private Locale locale;

  private Set<String> roles = new TreeSet<String>();

  private boolean retryOnFail;
  private String policyId;
  private boolean createCDR;
  private int smsPerSec;
  private Address sourceAddr;
  private Time deliveryStartTime;
  private Time deliveryEndTime;
  private List<Integer> deliveryDays = new ArrayList<Integer>();
  private Time validityPeriod;
  private Time messageTimeToLive;
  private DeliveryType deliveryType;
  private boolean transactionMode;
  private boolean allRegionsAllowed;
  private List<Integer> allowedRegions = new ArrayList<Integer>();
  private int priority;
  private boolean emailNotification;
  private boolean smsNotification;
  private boolean createArchive;
  private int deliveryLifetime;
  private boolean useDataSm;
  //private String directory;



  private final ValidationHelper vh = new ValidationHelper(User.class);
  public static final String INFORMER_ADMIN_ROLE = "informer-admin";
  public static final String INFORMER_USER_ROLE = "informer-user";

  private String cdrDestination;
  private String cdrOriginator;

  private List<UserCPsettings> cpSettings;

  private Charset fileCharset = Charset.forName("windows-1251");

  public User() {
  }

  public User(User user) {
    this.login = user.login;
    this.password = user.password;
    this.roles = user.getRoles() == null ? null : new TreeSet<String>(user.getRoles());
    this.firstName = user.firstName;
    this.lastName = user.lastName;
    this.phone = user.phone == null ? null : new Address(user.phone);
    this.email = user.email;
    this.status = user.status;
    this.organization = user.organization;
    this.locale = user.locale == null ? null : new Locale(user.locale.getLanguage());
    this.smsPerSec = user.smsPerSec;
    this.createCDR = user.createCDR;
    this.sourceAddr = new Address(user.sourceAddr);
    this.deliveryStartTime = user.deliveryStartTime == null ? null : new Time(user.getDeliveryStartTime());
    this.deliveryEndTime = user.deliveryEndTime == null ? null : new Time(user.getDeliveryEndTime());
    this.deliveryDays = new ArrayList<Integer>(user.getDeliveryDays());
    this.validityPeriod = user.validityPeriod;
    this.messageTimeToLive = user.messageTimeToLive;
    this.deliveryType = user.deliveryType;
    this.transactionMode = user.transactionMode;
    this.policyId = user.policyId;
    this.allowedRegions = user.getRegions() == null ? null : new ArrayList<Integer>(user.getRegions());
    this.priority = user.priority;
    this.emailNotification = user.emailNotification;
    this.smsNotification = user.smsNotification;
    this.createArchive=user.createArchive;
    this.deliveryLifetime=user.deliveryLifetime;
    this.useDataSm = user.useDataSm;

    this.allRegionsAllowed=user.allRegionsAllowed;
    this.retryOnFail = user.retryOnFail;

    this.cdrDestination = user.cdrDestination;
    this.cdrOriginator = user.cdrOriginator;

    this.cpSettings = user.getCpSettings()==null ? null : new ArrayList<UserCPsettings>(user.getCpSettings());
    this.fileCharset = user.getFileCharset();

  }

  public Charset getFileCharset() {
    return fileCharset;
  }

  public void setFileCharset(Charset fileCharset) {
    this.fileCharset = fileCharset;
  }

  public String getCdrOriginator() {
    return cdrOriginator;
  }

  public void setCdrOriginator(String cdrOriginator) {
    this.cdrOriginator = cdrOriginator;
  }

  public String getCdrDestination() {
    return cdrDestination;
  }

  public void setCdrDestination(String cdrDestination) {
    this.cdrDestination = cdrDestination;
  }

  public String getLogin() {
    return login;
  }

  public void setLogin(String login) {
    this.login = login;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public Set<String> getRoles() {
    return roles;
  }

  public void setRoles(Set<String> roles){
    this.roles = roles;
  }

  public String getFirstName() {
    return firstName;
  }

  public void setFirstName(String firstName) {
    this.firstName = firstName;
  }

  public String getLastName() {
    return lastName;
  }

  public void setLastName(String lastName) {
    this.lastName = lastName;
  }


  public Address getPhone() {
    return phone;
  }

  public void setPhone(Address phone) {
    this.phone = phone;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) {
    this.email = email;
  }

  public boolean hasRole(String name) {
    return roles.contains(name);
  }

  public void setStatus(Status status) {
    this.status = status;
  }

  public Status getStatus() {
    return status;
  }

  public void setOrganization(String organization) {
    this.organization = organization;
  }

  public String getOrganization() {
    return organization;
  }

  public void setLocale(Locale locale) {
    this.locale = locale;
  }

  public Locale getLocale() {
    return locale;
  }

  public boolean isCreateCDR() {
    return createCDR;
  }

  public void setCreateCDR(boolean createCDR) {
    this.createCDR = createCDR;
  }

  public int getSmsPerSec() {
    return smsPerSec;
  }

  public void setSmsPerSec(int smsPerSec) {
    this.smsPerSec = smsPerSec;
  }

  public Address getSourceAddr() {
    return sourceAddr;
  }

  public void setSourceAddr(Address sourceAddr) {
    this.sourceAddr = sourceAddr;
  }

  public Time getDeliveryStartTime() {
    return deliveryStartTime;
  }

  public void setDeliveryStartTime(Time deliveryStartTime) {
    this.deliveryStartTime = deliveryStartTime;
  }

  public Time getDeliveryEndTime() {
    return deliveryEndTime;
  }

  public void setDeliveryEndTime(Time deliveryEndTime) {
    this.deliveryEndTime = deliveryEndTime;
  }

  public List<Integer> getDeliveryDays() {
    return deliveryDays;
  }

  public Time getValidityPeriod() {
    return validityPeriod;
  }


  public void setValidityPeriod(Time validityPeriod) {
    this.validityPeriod = validityPeriod;
  }

  public void setDeliveryDays(List<Integer> deliveryDays)  {
    this.deliveryDays = deliveryDays == null ? new ArrayList<Integer>(0) : new ArrayList<Integer>(deliveryDays);
  }

  public DeliveryType getDeliveryType() {
    return deliveryType;
  }

  public void setDeliveryType(DeliveryType deliveryType) {
    this.deliveryType = deliveryType;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public String getPolicyId() {
    return policyId;
  }

  public void setPolicyId(String policyId) {
    this.policyId = policyId;
  }

  public List<Integer> getRegions() {
    return allowedRegions;
  }

  public void setRegions(List<Integer> regions){
    if (regions == null) {
      this.allowedRegions = null;
      return;
    }
    this.allowedRegions = new ArrayList<Integer>(regions);
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public boolean isEmailNotification() {
    return emailNotification;
  }

  public void setEmailNotification(boolean emailNotification) {
    this.emailNotification = emailNotification;
  }

  public boolean isSmsNotification() {
    return smsNotification;
  }

  public void setSmsNotification(boolean smsNotification) {
    this.smsNotification = smsNotification;
  }

  public boolean isCreateArchive() {
    return createArchive;
  }

  public void setCreateArchive(boolean createArchive) {
    this.createArchive = createArchive;
  }

  public int getDeliveryLifetime() {
    return deliveryLifetime;
  }

  public void setDeliveryLifetime(int deliveryLifetime) {
    this.deliveryLifetime = deliveryLifetime;
  }

  public Time getMessageTimeToLive() {
    return messageTimeToLive;
  }

  public void setMessageTimeToLive(Time messageTimeToLive) {
    this.messageTimeToLive = messageTimeToLive;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public boolean isAllRegionsAllowed() {
    return allRegionsAllowed;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public void setAllRegionsAllowed(boolean allRegionsAllowed) {
    this.allRegionsAllowed = allRegionsAllowed;
  }

  public List<UserCPsettings> getCpSettings() {
    //with clone
    if(cpSettings==null) return null;
    List<UserCPsettings> ret = new ArrayList<UserCPsettings>();
    for(UserCPsettings ucps : cpSettings) {
      ret.add(new UserCPsettings(ucps));
    }
    return ret;
  }

  public void setCpSettings(List<UserCPsettings> cpSettings) {
    //with clone
    if(cpSettings==null) this.cpSettings=null;
    else {
      this.cpSettings = new ArrayList<UserCPsettings>();
      for(UserCPsettings ucps : cpSettings) {
        this.cpSettings .add(new UserCPsettings(ucps));
      }
    }
  }

  public static final Pattern emailPattern = Pattern.compile("^[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*@[A-Za-z0-9]+[\\.\\-_A-Za-z0-9!#$&'*+/=?^_`{|}~:]*$");

  void validate() throws AdminException{
    vh.checkNotEmpty("login", login);
    vh.checkNotContains("login", login, ".");
    vh.checkNotEmpty("password", password);
    vh.checkNotNull("roles", roles);
    vh.checkNotEmpty("firstName", firstName);
    vh.checkNotEmpty("lastName", lastName);
    vh.checkNotNull("phone", phone);
    vh.checkNotEmpty("email", email);
    vh.checkMatches("email", email, emailPattern);
    vh.checkNotNull("status", status);
    vh.checkNotEmpty("organization", organization);
    vh.checkPositive("smsPerSec", smsPerSec);
    vh.checkNotNull("sourceAddr", sourceAddr);
    vh.checkNotNull("deliveryStartTime",deliveryStartTime);
    vh.checkLessThan("deliveryEndTime", deliveryStartTime, new Time(24,0,0));
    vh.checkNotNull("deliveryEndTime",deliveryEndTime);
    vh.checkLessThan("deliveryEndTime", deliveryEndTime, new Time(24,0,0));
    if (validityPeriod != null)
      vh.checkGreaterThan("validityPeriod", validityPeriod, new Time(0,0,59));
    if (messageTimeToLive != null)
      vh.checkGreaterThan("messageTimeToLive", messageTimeToLive, new Time(0,0,59));

    if (deliveryDays != null) {
      for (Integer day1 : deliveryDays) {
        vh.checkNotNull("deliveryDays", day1);
        vh.checkBetween("deliveryDays", day1, 0, 6);
        int count = 0;
        for(Integer day2 : deliveryDays) {
          vh.checkNotNull("deliveryDays", day2);
          if(day1.equals(day2)) {
            count++;
          }
        }
        vh.checkTrue("deliveryDays", count == 1);
      }
    }


    vh.checkNotNull("deliveryType", deliveryType);
    if (policyId != null && policyId.trim().length() == 0) policyId = null;

    if(allowedRegions != null) {
      for (Integer r : allowedRegions) {
        vh.checkNotNull("regions", r);
        int count = 0;
        for(Integer r2 : allowedRegions) {
          vh.checkNotNull("regions", r2);
          if(r.equals(r2)) {
            count++;
          }
        }
        vh.checkTrue("allowedRegions", count == 1);
      }
    }

    if(createArchive) {
      vh.checkPositive("deliveryLifetime", deliveryLifetime);
    }
    vh.checkBetween("priority", priority, 1, 100);

    if(cpSettings != null) {
      for(UserCPsettings s : cpSettings) {
        s.checkValid();
      }
    }

  }

  public enum Status {
    ENABLED,
    DISABLED
  }

  public enum DeliveryType {
    SMS, USSD_PUSH, USSD_PUSH_VIA_VLR
  }
}