
How to install Solr (Debian), 'the quick short version'.

Requirements;

- Java Development Kit (JDK) 1.5 or later.
- Any java EE servlet engine app-server. (jetty is included in solr). 
  Will be using apache-tomcat instead.


(as root)
 * Install JDK:

apt-get install sun-java6-jdk


(as root)
* Install tomcat6

apt-get install tomcat6

* Download & install latest solr;

wget http://apache.mirror.versatel.nl//lucene/solr/1.4.1/apache-solr-1.4.1.tgz
tar -zxf apache-solr-1.4.1.tgz

(as root)
* Setup solr under tomcat;

cp apache-solr-1.4.1/dist/apache-solr-1.4.1.war /usr/share/tomcat6/webapps/

(as root)
* Setup solr home using example as template;

cp -r apache-solr-1.4.1/example/solr/ /usr/share/tomcat6/solr


* create our config file
(as root)
nano /etc/tomcat6/Catalina/localhost/solr.xml

And paste;

<Context docBase="/usr/share/tomcat6/webapps/solr.war" debug="0" privileged="true" allowLinking="true" crossContext="true">
<Environment name="solr/home" type="java.lang.String" value="/usr/share/tomcat6/solr" override="true" />
</Context>


