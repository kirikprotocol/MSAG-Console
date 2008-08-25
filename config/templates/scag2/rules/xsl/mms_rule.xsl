<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:whoisd="http://www.sibinco.com/WHOISD"
                version="1.0">
   <xsl:template match="/">
       <xsl:copy-of select = "whoisd:terms-rules/whoisd:mms-rule/*"/>
   </xsl:template>
</xsl:stylesheet>