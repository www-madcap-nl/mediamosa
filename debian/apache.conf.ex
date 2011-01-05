Alias /mediamosa /usr/share/mediamosa

<Directory /usr/share/mediamosa/>
        Options +FollowSymLinks
        AllowOverride All
        order allow,deny
        allow from all
</Directory>

Alias /mediamosa/ticket /srv/mediamosa/links

<Directory /srv/mediamosa/links>
        Options FollowSymLinks
        AllowOverride All
        Order deny,allow
        Allow from All
</Directory>

<IfModule mod_php5.c>
        php_admin_value post_max_size 2008M
        php_admin_value upload_max_filesize 2000M
        php_admin_value memory_limit 64M
</IfModule>
