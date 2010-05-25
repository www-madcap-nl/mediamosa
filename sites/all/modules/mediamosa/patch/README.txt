Notes on .patch files.

These patch files are applied on Drupal 7 and are required in some cases.
MediaMosa is released with Drupal 7 and has these patches applied. These patches
are used for own Drupal 7 versions, when needed for upgrading Drupal. However
remember that its best to use the Drupal version supplied with MediaMosa.

- cookiedomain.patch
This patch is required for the cookie domain fix and the fix on simpletest. The
cookie domain fix will fix problems on the 'www.' subdomain usage of Drupal.
Patch will allow you to use cookies on different subdomains by 

- simpletest.patch
Drupal 7 has problems when running simpletest on loadbalencers while tests can
do HTTP requests inside a simpletest. When during the HTTP request inside the
test is quering the loadbalencer, the other server might be chosen and resulting
in a 403 (forbidden) http error. This patch will fix this problem.

This patch also fixes the problem on having big log files for master-slave
setups for MySQL. The fix will enable you to choose a different database where
to run your simpletest, thus allowing you to ignore this database with mysql
for replication. View your /etc/mysql/my.cnf how to ingore your simpletest
datebase.
