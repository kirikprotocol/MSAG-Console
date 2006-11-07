package ru.sibinco.automation;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: 20.10.2006
 *
 * Это прога, которая конвертирует CSV файл с данными о соответствии номеров
 * СМС центров и номеров абонентов. Сам CSV файл может быть получен из XLS файла через MSExcel.
 * Пример XLS файла и полученного из него CVS в папке doc.
 * Прога извлекает из CSV файла маски абонентов, раскрывает эти маски и строит по полученным
 * номерам конечныq автомат, распознающий их (для каждого региона свой автомат).
 * Далее эти автоматы сохраняются в выходном файле.
 */

public class MasksListConvertor {

  private static MacroRegion macroRegion = null;
  private static String macroRegionName = null;
  private static ArrayList macroRegions = new ArrayList();

  public static void main(String[] args) {
    if (args.length < 2) {
      System.out.println("Programm must have 2 parameters: input csv file and output xml file");
      return;
    }


    int lineNumber = 0;
    String cline = "";
    try {
      System.out.println("Read incoming file...");

      final BufferedReader br = new BufferedReader(new FileReader(args[0]));
      lineNumber = 1;
      cline = br.readLine(); // admit title 1
      lineNumber = 2;
      cline = br.readLine(); // admit title 2

      StringTokenizer st;
      String line;
      String token1, token2, token3, token4;
      while((line = br.readLine()) != null) {
        // Add ' ' before every ';'
        StringBuffer newLine = new StringBuffer();
        for (int i=0; i<line.length(); i++) {
          if (line.charAt(i) == ';')
            newLine.append(" ");
          newLine.append(line.charAt(i));
        }

        lineNumber++;
        cline = line;
        st = new StringTokenizer(newLine.toString(), ";");
        token1 = (st.hasMoreTokens()) ? st.nextToken() : "";
        token2 = (st.hasMoreTokens()) ? st.nextToken() : "";
        token3 = (st.hasMoreTokens()) ? st.nextToken() : "";
        token4 = (st.hasMoreTokens()) ? st.nextToken() : "";
        processString(token1.trim(), token2.trim(), token3.trim(), token4.trim());
      }

      br.close();
      System.out.println("Read incoming file ok.");

    } catch (FileNotFoundException e) {
      e.printStackTrace();
      return;
    } catch (Exception e) {
      System.out.println("Error reading line No " + String.valueOf(lineNumber) + ", line=" + cline);
      e.printStackTrace();
      return;
    }

    System.out.println("");
    System.out.println("");
    System.out.println("");

    try {
      System.out.println("Process data and create outpit file...");
      for (Iterator iter = macroRegions.iterator(); iter.hasNext();)
        ((MacroRegion)iter.next()).createAutomation();

      final BufferedWriter writer = new BufferedWriter(new FileWriter(args[1]));

      writer.write("<?xml version=\"1.0\" encoding=\"windows-1251\"?>");
      writer.write("<templates>\n");

      for (Iterator iter = macroRegions.iterator(); iter.hasNext();)
        ((MacroRegion)iter.next()).save(writer);

      writer.write("</templates>");
      writer.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  private static void processString(String token1, String token2, String token3, String token4) {
    if (token1.equals("")&&token2.equals("")&&token3.equals(""))
      return;

    if (token2.equals("") && token3.equals("")) {// Macro region
      macroRegionName = token1;
      return;
    }

    if (macroRegionName != null) {
      macroRegion = new MacroRegion(macroRegionName, token4);
      macroRegions.add(macroRegion);
      macroRegionName = null;
      processNumbersToken(token2, false);
      return;
    }

    processNumbersToken(token2, true);
  }

  private static void processNumbersToken(String token, boolean removeAllSpaces) {
    // Remove spaces before chars '-' and ','
    StringBuffer result = new StringBuffer();
    boolean sgBefore = false;
    for (int i=0; i < token.length(); i++) {
      if (token.charAt(i) == ' ') {
        while (token.charAt(i) == ' ')
          i++;
        if (token.charAt(i) != '-' && token.charAt(i)!=',' && !sgBefore && !removeAllSpaces)
          result.append(" ");
      }

      sgBefore = false;
      result.append(token.charAt(i));

      if (token.charAt(i) == '-' || token.charAt(i) == ',')
        sgBefore = true;
    }

    final StringTokenizer st = new StringTokenizer(result.toString().trim(), " ");

    while (st.hasMoreTokens())
      processNumberMask(st.nextToken());
  }

  private static void processNumberMask(String numberMask) {
    // Remove unneccessary chars
    StringBuffer resultMask = new StringBuffer();
    for (int i=0; i< numberMask.length(); i++) {
      final char ch=numberMask.charAt(i);
      if (Character.isDigit(ch) || ch == '-' || ch ==',')
        resultMask.append(ch);
    }

    String result = resultMask.toString();
    // Remove ',' at the and
    if (result.length() > 0 && (result.charAt(result.length()-1) == ',' || result.charAt(result.length()-1) == '-'))
      result = result.substring(0, result.length() - 1);

    if (resultMask.toString().matches("\\d+.*"))
      addNumbersFromMask(result);
  }

  private static void addNumbersFromMask(String mask) {
    macroRegion.addNumbers(new Mask(mask).getNumbers());
  }

  private static class MacroRegion {
    private final String name;
    private final String number;
    private final Set numbers = new TreeSet();
    private Automation a = null;

    public MacroRegion(final String name, final String number) {
      this.name = name;
      this.number = number;
    }

    public void addNumbers(Collection numbers) {
      this.numbers.addAll(numbers);
    }

    public void createAutomation() {
      System.out.println("=============================================================================================");
      System.out.println("Create automation for macro region " + name + "...");
      System.out.println("Numbers count = " + numbers.size() + ".");
      a = AutomationCreator.createAutomation(new ArrayList(numbers));
      System.out.println("");
      System.out.println("Automation created. Size = " + a.size() + ". Check ...");

      { // Check template
        boolean ok = true;
        long start = System.currentTimeMillis();
        for (Iterator it = numbers.iterator(); it.hasNext();){
          String word = (String)it.next();
          if (!a.allowWord(word)) {
            System.out.println(word + " is not allowed by automation!");
            ok = false;
            break;
          }
        }
        long end = System.currentTimeMillis();
        System.out.println("Check result = " + ok + ". Checking time = " + String.valueOf(end-start));
        System.out.println("Allow my number: " + a.allowWord("9607891901"));
      }
    }

    public void save(Writer writer) throws IOException {
      if (a != null) {
        // Create template
        System.out.println("=============================================================================================");
        System.out.println("Store macro region " + name + "...");

        // Save data
        System.out.println("Save automation...");
        writer.write("<MacroRegion name=\"" + name + "\" number=\"" + number + "\">\n");
        a.save(writer);
        writer.write("</MacroRegion>\n");
        System.out.println("Data saved.");
      } else
        System.out.println("!!! Automation for macro region " + name + " is null !!!");
    }
  }
}
