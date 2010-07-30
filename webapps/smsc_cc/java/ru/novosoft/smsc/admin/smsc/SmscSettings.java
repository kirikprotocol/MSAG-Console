package ru.novosoft.smsc.admin.smsc;

/**
 * @author Artem Snopkov
 */
public class SmscSettings {

  private CommonSettings commonSettings;

  private InstanceSettings[] instanceSettings;

  SmscSettings() {
  }

  SmscSettings(SmscSettings c) {
    this.commonSettings = new CommonSettings(c.commonSettings);
    this.instanceSettings = new InstanceSettings[c.instanceSettings.length];

    for (int i = 0; i<c.instanceSettings.length; i++)
      this.instanceSettings[i] = new InstanceSettings(c.instanceSettings[i]);
  }

  public CommonSettings getCommonSettings() {
    return commonSettings;
  }

  public void setCommonSettings(CommonSettings commonSettings) {
    this.commonSettings = commonSettings;
  }

  public int getSmscInstancesCount() {
    return instanceSettings.length;
  }

  public InstanceSettings getInstanceSettings(int instanceNumber) {
    if (instanceNumber < 0 || instanceNumber > instanceSettings.length - 1) {
      throw new IllegalArgumentException("Illegal instance number: " + instanceNumber + ". Must be in [0," + (instanceSettings.length - 1) + "]");
    }
    return instanceSettings[instanceNumber];
  }

  public void setInstanceSettings(int instanceNumber, InstanceSettings instanceSettings) {
    if (instanceNumber < 0 || instanceNumber > this.instanceSettings.length - 1) {
      throw new IllegalArgumentException("Illegal instance number: " + instanceNumber + ". Must be in [0," + (this.instanceSettings.length - 1) + "]");
    }

    this.instanceSettings[instanceNumber] = instanceSettings;
  }

  void setInstancesSettings(InstanceSettings[] settings) {
    this.instanceSettings = settings;
  }

  public SmscSettings cloneSettings() {
    return new SmscSettings(this);
  }
}
