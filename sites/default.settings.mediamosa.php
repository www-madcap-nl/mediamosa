<?php
$databases = array();
$databases['default']['default'] = array(
  'driver' => 'mysql',
  'database' => 'mediamosa2',
  'username' => 'memo',
  'password' => 'memo',
  'host' => 'localhost'
);

$databases['ftp']['default'] = array(
  'driver' => 'mysql',
  'database' => 'vpx_ftp',
  'username' => 'ftp',
  'password' => 'vpx',
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
 * CMS admin interface. You can turn on the admin with app interface
 * but remember that some url like /user conflicts between the CMS and
 * app interface.
 */
$conf['mediamosa_admin'] = TRUE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * scheduling of the jobs. Only one installation should be enabled.
 */
$conf['mediamosa_job_schedular'] = TRUE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * processing of the jobs. You can specifiy unlimited amount of 
 * installation for job processing.
 */
$conf['mediamosa_job_processor'] = TRUE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * MediaMosa background monitor.
 */
$conf['mediamosa_monitor'] = TRUE;

//define('MEDIAMOSA_REST_HOST', 'localhost');
//define('MEDIAMOSA_BUILD_URL', '');
//define('MEDIAMOSA_URL', 'http://' . MEDIAMOSA_REST_HOST . '/' . MEDIAMOSA_BUILD_URL);
