package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;

import java.io.Serializable;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.TreeSet;

/**
 * Настройки пользователя
 * @author Aleksandr Khalitov
 */
public class User implements Serializable{

  private String login;
  private String password;
  private Status status;
  private String firstName;
  private String lastName;
  private String phone;
  private String email;
  private String organization;
  private Locale locale;
  private Set<String> roles = new TreeSet<String>();


  private String policyId;
  private boolean createCDR=true;
  private int smsPerSec=1;
  private Address sourceAddr;
  private Time   deliveryStartTime;
  private Time   deliveryEndTime;
  private Set<Integer> deliveryDays = new TreeSet<Integer>();
  private int validHours;
  private DeliveryType deliveryType;
  private boolean transactionMode;
  private Set<String> regions = new TreeSet<String>();
  private int priority;
  private boolean emailNotification;
  private boolean smsNotification;
  private boolean createArchive;
  private int deliveryLifetime;

  private String directory;
  private int directoryPoolPeriod;
  private boolean createReports;
  private int reportsLifetime;

  private final ValidationHelper vh = new ValidationHelper(User.class);

  public User() {
  }

  public User(User user) {
    this.login = user.login;
    this.password = user.password;
    this.roles = user.getRoles() == null ? null : new TreeSet<String>(user.getRoles());
    this.firstName = user.firstName;
    this.lastName = user.lastName;
    this.phone = user.phone;
    this.email = user.email;
    this.status = user.status;
    this.organization=user.organization;
    this.locale = user.locale == null ? null : new Locale(user.locale.getLanguage());
    this.smsPerSec=user.smsPerSec;
    this.createCDR = user.createCDR;
    this.sourceAddr = new Address(user.sourceAddr);
    this.deliveryStartTime = user.deliveryStartTime == null ? null : new Time(user.getDeliveryStartTime());
    this.deliveryEndTime   = user.deliveryEndTime == null ? null : new Time(user.getDeliveryStartTime());
    this.deliveryDays = user.getDeliveryDays() == null ? null : new TreeSet<Integer>(user.getDeliveryDays());
    this.validHours = user.validHours;
    this.deliveryType = user.deliveryType;
    this.transactionMode=user.transactionMode;
    this.policyId = user.policyId;
    this.regions = user.getRegions() == null ? null : new TreeSet<String>(user.getRegions());
    this.priority = user.priority;
    this.emailNotification = user.smsNotification;
    this.smsNotification = user.smsNotification;
    this.createArchive=user.createArchive;
    this.deliveryLifetime=user.deliveryLifetime;
    this.directory=user.directory;
    this.directoryPoolPeriod=user.directoryPoolPeriod;
    this.createReports=user.createReports;
    this.reportsLifetime=user.reportsLifetime;

  }


  public String getLogin() {
    return login;
  }

  public void setLogin(String login) throws AdminException {
    vh.checkNotEmpty("login", login);
    this.login = login;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) throws AdminException{
    vh.checkNotEmpty("password", password);
    this.password = password;
  }

  public Set<String> getRoles() {
    return roles;
  }

  public void setRoles(Set<String> roles) throws AdminException {
    vh.checkNotNull("roles", roles);
    this.roles = roles;
  }

  public String getFirstName() {
    return firstName;
  }

  public void setFirstName(String firstName) throws AdminException {
    vh.checkNotEmpty("firstName", firstName);
    this.firstName = firstName;
  }

  public String getLastName() {
    return lastName;
  }

  public void setLastName(String lastName) throws AdminException {
    vh.checkNotEmpty("lastName", lastName);
    this.lastName = lastName;
  }


  public String getPhone() {
    return phone;
  }

  public void setPhone(String phone) throws AdminException {
    vh.checkNotEmpty("phone", phone);
    this.phone = phone;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) throws AdminException {
    vh.checkNotEmpty("email", email);
    this.email = email;
  }

  public boolean hasRole(String name) {
    return roles.contains(name);
  }

  public void setStatus(Status status) throws AdminException {
    vh.checkNotNull("status", status);
    this.status = status;
  }

  public Status getStatus() {
    return status;
  }

  public void setOrganization(String organization) throws AdminException {
    vh.checkNotEmpty("organization", organization);
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

  public void setSmsPerSec(int smsPerSec) throws AdminException {
    vh.checkPositive("smsPerSec",smsPerSec);
    this.smsPerSec = smsPerSec;
  }

  public Address getSourceAddr() {
    return sourceAddr;
  }

  public void setSourceAddr(Address sourceAddr) throws AdminException {
    vh.checkNotNull("sourceAddr",sourceAddr);
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

  public Set<Integer> getDeliveryDays() {
    return deliveryDays;
  }

  public int getValidHours() {
    return validHours;
  }

  public void setValidHours(int validHours) throws AdminException {
    vh.checkPositive("validHours",validHours);
    this.validHours = validHours;
  }

  public void setDeliveryDays(Set<Integer> deliveryDays) throws AdminException {
    Set<Integer> deliveryDaysOut = new TreeSet<Integer>();
    if(deliveryDays!=null) {
      for(Integer day : deliveryDays) {
        vh.checkNotContains("deliveryDays",deliveryDaysOut,day);
        vh.checkBetween("deliveryDays",day,new Integer(0),new Integer(6));
        deliveryDaysOut.add(day);
      }
    }
    this.deliveryDays = deliveryDaysOut;
  }

  public DeliveryType getDeliveryType() {
    return deliveryType;
  }

  public void setDeliveryType(DeliveryType deliveryType) throws AdminException {
    vh.checkNotNull("deliveryType",deliveryType);
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

  public void setPolicyId(String policyId) throws AdminException {
    vh.checkNotEmpty("policyId",policyId);
    this.policyId = policyId;
  }

  public Set<String> getRegions() {

    return regions;
  }

  public void setRegions(Set<String> regions) throws AdminException {
    vh.checkNotNull("regions",regions);
    for(String r : regions) {
      vh.checkNotEmpty("regions",r);
    }
    this.regions = regions;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) throws AdminException {
    vh.checkPositive("priority",priority);
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

  public void setDeliveryLifetime(int deliveryLifetime) throws AdminException {
    vh.checkPositive("deliveryLifetime",deliveryLifetime);
    this.deliveryLifetime = deliveryLifetime;
  }

  public String getDirectory() {
    return directory;
  }

  public void setDirectory(String directory) throws AdminException {
    vh.checkNotEmpty("directory",directory);
    this.directory = directory;
  }

  public int getDirectoryPoolPeriod() {
    return directoryPoolPeriod;
  }

  public void setDirectoryPoolPeriod(int directoryPoolPeriod) throws AdminException {
    vh.checkPositive("directoryPoolPeriod",directoryPoolPeriod);
    this.directoryPoolPeriod = directoryPoolPeriod;
  }

  public boolean isCreateReports() {
    return createReports;
  }

  public void setCreateReports(boolean createReports) {
    this.createReports = createReports;
  }

  public int getReportsLifetime() {
    return reportsLifetime;
  }

  public void setReportsLifetime(int reportsLifetime) throws AdminException {
    vh.checkPositive("reportsLifetime",reportsLifetime);
    this.reportsLifetime = reportsLifetime;
  }

  public enum Status {
    ENABLED,
    DISABLED
  }

  public enum DeliveryType {
    SMS, USSD_PUSH, USSD_PUSH_VIA_VLR
  }
}
