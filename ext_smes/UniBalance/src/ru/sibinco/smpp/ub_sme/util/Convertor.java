package ru.sibinco.smpp.ub_sme.util;

/**
 * User: pasha
 * Date: 28.11.2007
 * Time: 13:16:37
 */
public class Convertor {
    private final static String[] translit = {
            "A", "B", "V", "G", "D", "E", "Zh", "Z", "I", "J", "K", "L", "M", "N", "O",
            "P", "R", "S", "T", "U", "F", "H", "C", "Ch", "Sh", "Shh", "'", "Y", "'",
            "Je", "Ju", "Ja", "a", "b", "v", "g", "d", "e", "zh", "z", "i", "j", "k",
            "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "h", "c", "ch", "sh",
            "shh", "'", "y", "'", "je", "ju", "ja"
    };

    public static String translit(String s) {
        if (s == null) {
            return null;
        }
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (1040 <= c && c <= 1103) {
                sb.append(translit[c - 1040]);
            } else if (c == 1025) {
                sb.append('E');
            } else if (c == 1105) {
                sb.append('e');
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

}
