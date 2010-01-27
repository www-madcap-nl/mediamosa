<?php
$databases = array();
$databases['default']['default'] = array(
  'driver' => 'mysql',
  'database' => 'mediamosa2',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);

// Migration memo 1.0 databases;
$databases['mig_memo']['default'] = array(
  'driver' => 'mysql',
  'database' => 'memo',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);
$databases['mig_memo_data']['default'] = array(
  'driver' => 'mysql',
  'database' => 'memo_data',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);

/**
 * The 'mediamosa_installation_id' defines the default install ID
 * for multiple installations of mediamosa. If you use 2 or more
 * www installations of mediamosa for the same database, then
 * specifiy a server ID here. Max length is 16 chars. The
 * server ID can be anything
 */
$conf['mediamosa_installation_id'] = 'default';

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST interface. Putting it on FALSE will disable the interface
 * for this URL / Location, disabling all REST calls; except upload
 * REST calls, which is controlled by mediamosa_app_upload.
 */
$conf['mediamosa_app'] = FALSE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST upload interface. The upload setting allows REST call
 * relating to uploading files, it will not allow other REST calls
 * unless 'mediamosa_app' is TRUE also. To setup this interface
 * as an upload interface, put 'mediamosa_app' to FALSE and set
 * 'mediamosa_app_upload' to TRUE.
 */
$conf['mediamosa_app_upload'] = FALSE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST download interface. The download setting allows you
 * to download mediafile using tickets. For now its used to download
 * files and still images.
 * Warning: If your download servers point to the admin, then make sure
 * sure you allow this setting on the admin, else your MediaMosa status page
 * will show failures.
 */
$conf['mediamosa_app_download'] = TRUE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * CMS admin interface. You can turn on the admin with app interface
 * but remember that some url like /user conflicts with the drupal
 * /user and the /user mediamosa.
 */
$conf['mediamosa_admin'] = TRUE;

