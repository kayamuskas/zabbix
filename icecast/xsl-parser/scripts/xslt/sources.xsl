<xsl:stylesheet xmlns:xsl = "http://www.w3.org/1999/XSL/Transform" version = "1.0">
 <xsl:output method="text"/>
 <xsl:template match = "/icestats">
  <xsl:value-of select="sources/text()"/>
 </xsl:template>
</xsl:stylesheet>
