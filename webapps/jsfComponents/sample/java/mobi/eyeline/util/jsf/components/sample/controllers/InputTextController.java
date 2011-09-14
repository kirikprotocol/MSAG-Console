package mobi.eyeline.util.jsf.components.sample.controllers;

/**
 * author: Aleksandr Khalitov
 */
public class InputTextController {

  private String stringValue;

  private Integer intValue;

  private String required;

  private String size;

  private String readonly = "Readonly!";

  private StringWrapper converter = new StringWrapper("dasdsada!");

  private String textarea;

  private String validatelenght;

  private Integer formatSetter;

  private Integer illegalSetter;

  private Inner inner = new Inner();

  public String getFormatSetter() {
    return formatSetter == null ? null : formatSetter.toString();
  }

  public void setFormatSetter(String formatSetter) {
    this.formatSetter = formatSetter == null || formatSetter.length() == 0 ? 0 : Integer.parseInt(formatSetter);
  }

  public String getIllegalSetter() {
    return illegalSetter == null ? null : illegalSetter.toString();
  }

  public void setIllegalSetter(String illegalSetter) {
    try{
    this.illegalSetter = illegalSetter == null || illegalSetter.length() == 0 ? 0 : Integer.parseInt(illegalSetter);
    }catch (NumberFormatException e) {
      throw new IllegalArgumentException("Can't set property", e);
    }
  }

  public String getTextarea() {
    return textarea;
  }

  public void setTextarea(String textarea) {
    this.textarea = textarea;
  }

  public String getValidatelenght() {
    return validatelenght;
  }

  public void setValidatelenght(String validatelenght) {
    this.validatelenght = validatelenght;
  }

  public StringWrapper getConverter() {
    return converter;
  }

  public void setConverter(StringWrapper converter) {
    this.converter = converter;
  }

  public String getReadonly() {
    return readonly;
  }

  public void setReadonly(String readonly) {
    this.readonly = readonly;
  }
  public String getSize() {
    return size;
  }

  public void setSize(String size) {
    this.size = size;
  }

  public String getRequired() {
    return required;
  }

  public void setRequired(String required) {
    this.required = required;
  }

  public String getStringValue() {
    return stringValue;
  }

  public void setStringValue(String stringValue) {
    this.stringValue = stringValue;
  }

  public Integer getIntValue() {
    return intValue;
  }

  public void setIntValue(Integer intValue) {
    this.intValue = intValue;
  }

  public Inner getInner() {
    return inner;
  }

  public void setInner(Inner inner) {
    this.inner = inner;
  }

  public static class Inner {
    private int innerInt;

    public Inner() {
    }

    public int getInnerInt() {
      return innerInt;
    }

    public void setInnerInt(int innerInt) {
      this.innerInt = innerInt;
    }
  }
}
