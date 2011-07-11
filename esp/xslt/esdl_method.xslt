<?xml version="1.0" encoding="UTF-8"?>
<!--

## Copyright © 2011 HPCC Systems.  All rights reserved.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:ipo="http://www.altova.com/IPO" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:n1="http://www.xmlspy.com/schemas/orgchart">
    <xsl:output version="1.0" encoding="UTF-8" indent="no" omit-xml-declaration="no" media-type="text/html"/>
    <xsl:template match="/">
        <html>
            <head>
                <title/>
        <link rel="stylesheet" type="text/css" href="/esp/files/yui/build/fonts/fonts-min.css" />
        <link rel="stylesheet" type="text/css" href="/esp/files/css/espdefault.css" />
      </head>
      <body class="yui-skin-sam">
                SERVICE FILE:<xsl:value-of select="MainResponse/File"/>
                <br/>
                METHOD: <xsl:value-of select="MainResponse/EsdlMethod"/>
                <br/>
                <br/>
                <br/>
                <a>
                    <xsl:attribute name="href"><xsl:text disable-output-escaping="yes">ViewESDL?File=</xsl:text><xsl:value-of select="MainResponse/File"/><xsl:text disable-output-escaping="yes">&amp;EsdlMethod=</xsl:text><xsl:value-of select="MainResponse/EsdlMethod"/></xsl:attribute>View the ESDL definition in XML format
                </a>
                <br/>
                <a>
                    <xsl:attribute name="href"><xsl:text disable-output-escaping="yes">GetMethodSchema?File=</xsl:text><xsl:value-of select="MainResponse/File"/><xsl:text disable-output-escaping="yes">&amp;EsdlMethod=</xsl:text><xsl:value-of select="MainResponse/EsdlMethod"/></xsl:attribute>View the XML Schema
                </a>
                <br/>
                <a>
                    <xsl:attribute name="href"><xsl:text disable-output-escaping="yes">ViewForm?File=</xsl:text><xsl:value-of select="MainResponse/File"/><xsl:text disable-output-escaping="yes">&amp;EsdlMethod=</xsl:text><xsl:value-of select="MainResponse/EsdlMethod"/></xsl:attribute>Preview the request FORM page
                </a>
                <br/>
                <a>
                    <xsl:attribute name="href"><xsl:text disable-output-escaping="yes">SampleMessage?Type=request&amp;File=</xsl:text><xsl:value-of select="MainResponse/File"/><xsl:text disable-output-escaping="yes">&amp;EsdlMethod=</xsl:text><xsl:value-of select="MainResponse/EsdlMethod"/></xsl:attribute>View the sample Request XML
                </a>
                <br/>
                <a>
                    <xsl:attribute name="href"><xsl:text disable-output-escaping="yes">SampleMessage?Type=response&amp;File=</xsl:text><xsl:value-of select="MainResponse/File"/><xsl:text disable-output-escaping="yes">&amp;EsdlMethod=</xsl:text><xsl:value-of select="MainResponse/EsdlMethod"/></xsl:attribute>View the sample Response XML
                </a>
                <br/>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>
