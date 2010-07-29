<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * MediaMosa installation profile.
 */

/**
 * @defgroup constants Module Constants
 * @{
 */

/**
 * Test text for Lua Lpeg
 */
define('MEDIAMOSA_TEST_LUA_LPEG', 'Usage: vpx-analyse BASE_PATH HASH [--always_hint_mp4] [--always_insert_metadata]');

/**
 * @} End of "defgroup constants".
 */

/**
 * Implementation of hook_form_alter().
 */
function mediamosa_profile_form_alter(&$form, $form_state, $form_id) {

  if ($form_id == 'install_configure_form') {
    // Set default for site name field.
    $form['site_information']['site_name']['#default_value'] = $_SERVER['SERVER_NAME'];
    $form['site_information']['site_mail']['#default_value'] = 'webmaster@' . $_SERVER['SERVER_NAME'];
    $form['admin_account']['account']['name']['#default_value'] = 'admin';
    $form['admin_account']['account']['mail']['#default_value'] = 'admin@' . $_SERVER['SERVER_NAME'];
  }
}

/**
 * Set up title.
 */
function _mediamosa_profile_get_title() {

  $inc = include_once (DRUPAL_ROOT . '/sites/all/modules/mediamosa/mediamosa.version.inc');

  // Try to include the settings file.
  $version = $inc ? mediamosa_version::get_current_version() : null;

  return 'Installing MediaMosa ' . ($version ? $version[mediamosa_version::MAJOR] . '.' .  $version[mediamosa_version::MINOR] . '.' . $version[mediamosa_version::RELEASE] . ' build ' . $version[mediamosa_version::BUILD] : '- unkwown version.');
}

/**
 * Implementation of hook_install_tasks().
 */
function mediamosa_profile_install_tasks() {

  drupal_set_title(_mediamosa_profile_get_title());

  $tasks = array(
    'mediamosa_profile_storage_location_form' => array(
      'display_name' => st('Storage location'),
      'type' => 'form',
      'run' => variable_get('mediamosa_current_mount_point', '') ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
    ),
    'mediamosa_profile_client_application_form' => array(
      'display_name' => st('Client application'),
      'type' => 'form',
    ),
    'mediamosa_profile_configure_server' => array(
      'display_name' => st('Configure the server'),
      //'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),
    'mediamosa_profile_cron_settings_form' => array(
      'display_name' => st('Cron & Apache settings'),
      'type' => 'form',
    ),
  );
  return $tasks;
}

/**
 * Implementation of hook_install_tasks_alter().
 */
function mediamosa_profile_install_tasks_alter(&$tasks, $install_state) {
  // Necessary PHP settings.
  // Should be first.
  $tasks = array(
    'mediamosa_profile_php_settings' => array(
      'display_name' => st('PHP settings'),
    ),
  ) + $tasks;
}


/**
 * Checking the settings.
 * Tasks callback.
 */
function mediamosa_profile_php_settings($install_state) {
  $output = '';
  $error = FALSE;


  // PHP modules.

  $output .= '<h1>' . t('PHP modules') . '</h1>';

  $required_extensions = array('bcmath', 'gd', /*'mcrypt',*/ 'curl', 'mysql', 'mysqli', 'SimpleXML');
  $modules = '';
  foreach ($required_extensions as $extension) {
    $modules .= $extension . ', ';
  }

  $output .= t('The following required modules are required: ') . substr($modules, 0, -2) . '.<br />';

  $loaded_extensions = array();
  $loaded_extensions = get_loaded_extensions();

  $missing = '';
  foreach ($required_extensions as $extension) {
    if (!in_array($extension, $loaded_extensions)) {
      $missing .= $extension . ', ';
    }
  }
  if (!empty($missing)) {
    drupal_set_message(t('The following required modules are missing: ') . substr($missing, 0, -2), 'error');
    $error = TRUE;
  }


  // Applications.

  $output .= '<h1>' . t('Required applications') . '</h1>';
  $output .= t('The following applications are required: ') . '<br />';
  $output .= '<ul>';

  // FFmpeg.
  $exec_output = array();
  exec('ffmpeg -version > /dev/null 2>&1', $exec_output, $ret_val);

  $output .= '<li>FFmpeg</li>';
  if ($ret_val != 0) {
    drupal_set_message(t('FFmpeg is not found. Please, install it first.'), 'error');
    $error = TRUE;
  }

  // Lua.
  $last_line = exec('lua 2>&1');
  $output .= '<li>Lua</li>';
  if ($last_line) {
    drupal_set_message(t('Lua is not found. Please, install it first.'), 'error');
    $error = TRUE;
  }

  // Lpeg.
  $last_line = exec('lua sites/all/modules/mediamosa/lib/lua/vpx-analyse 2>&1', $retval);
  $output .= '<li>Lua LPeg</li>';
  if ($last_line != MEDIAMOSA_TEST_LUA_LPEG) {
    drupal_set_message(t('Lpeg  extention of Lua is not found. Please, install it first.'), 'error');
    $error = TRUE;
  }

  $output .= '</ul>';


  // PHP ini.

  $output .= '<h1>' . t('Required settings') . '</h1>';

  $php_error_reporting = ini_get('error_reporting');
  if ($php_error_reporting & E_NOTICE)  {
    drupal_set_message(t('PHP error_reporting should be set at E_ALL & ~E_NOTICE.'), 'warning');
  }
  $output .= t('PHP apache error_reporting should be set at E_ALL & ~E_NOTICE.') . '<br />';

  $php_error_reporting = exec("php -r \"print(ini_get('error_reporting'));\"");
  if ($php_error_reporting & E_NOTICE)  {
    drupal_set_message(t('PHP cli error_reporting should be set at E_ALL & ~E_NOTICE.'), 'warning');
  }
  $output .= t('PHP client error_reporting should be set at E_ALL & ~E_NOTICE.') . '<br />';

  $php_upload_max_filesize = ini_get('upload_max_filesize');
  if ((substr($php_upload_max_filesize, 0, -1) < 100) &&
      (substr($php_upload_max_filesize, -1) != 'M' || substr($php_upload_max_filesize, -1) != 'G')) {
    drupal_set_message(t('upload_max_filesize should be at least 100M. Currently: %current_value', array('%current_value' => $php_upload_max_filesize)), 'warning');
  }
  $output .= t('upload_max_filesize should be at least 100M. Currently: %current_value', array('%current_value' => $php_upload_max_filesize)) . '<br />';

  $php_memory_limit = ini_get('memory_limit');
  if ((substr($php_memory_limit, 0, -1) < 128) &&
      (substr($php_memory_limit, -1) != 'M' || substr($php_memory_limit, -1) != 'G')) {
    drupal_set_message(t('memory_limit should be at least 128M. Currently: %current_value', array('%current_value' => $php_memory_limit)), 'warning');
  }
  $output .= t('memory_limit should be at least 128M. Currently: %current_value', array('%current_value' => $php_memory_limit)) . '<br />';

  $php_post_max_size = ini_get('post_max_size');
  if ((substr($php_post_max_size, 0, -1) < 100) &&
      (substr($php_post_max_size, -1) != 'M' || substr($php_post_max_size, -1) != 'G')) {
      drupal_set_message(t('post_max_size should be at least 100M. Currently: %current_value', array('%current_value' => $php_post_max_size)), 'warning');
  }
  $output .= t('post_max_size should be at least 100M. Currently: %current_value', array('%current_value' => $php_post_max_size)) . '<br />';
/*
  $php_max_input_time = ini_get('max_input_time');
  if ($php_max_input_time < 7200) {
    drupal_set_message(t('max_input_time should be at least 7200. Currently: %current_value', array('%current_value' => $php_max_input_time)), 'warning');
  }
  $output .= t('max_input_time should be at least 7200. Currently: %current_value', array('%current_value' => $php_max_input_time)) . '<br />';

  $php_max_execution_time = ini_get('max_execution_time');
  if ($php_max_execution_time < 7200) {
    drupal_set_message(t('max_execution_time should be at least 7200. Currently: %current_value', array('%current_value' => $php_max_execution_time)), 'warning');
  }
  $output .= t('max_execution_time should be at least 7200. Currently: %current_value', array('%current_value' => $php_max_execution_time)) . '<br />';
*/

  return $error ? $output : NULL;
}

/**
 * Get the mount point.
 * Task callback.
 */
function mediamosa_profile_storage_location_form() {
  $form = array();

  $mount_point = variable_get('mediamosa_current_mount_point', '/srv/mediamosa');
  $mount_point_windows = variable_get('mediamosa_current_mount_point_windows', '\\\\');

  $form['current_mount_point'] = array(
    '#type' => 'textfield',
    '#title' => t('Mediamosa SAN/NAS Mount point'),
    '#description' => t('The mount point is used to store the mediafiles.<br />Make sure the Apache user has write access to the Mediamosa SAN/NAS mount point.'),
    '#required' => TRUE,
    '#default_value' => $mount_point,
  );

  $form['current_mount_point_windows'] = array(
    '#type' => 'textfield',
    '#title' => t('Mediamosa SAN/NAS Mount point for Windows'),
    '#description' => t("The mount point is used to store the mediafiles.<br />Make sure the webserver has write access to the Windows Mediamosa SAN/NAS mount point.<br />If you don't use Windows, just leave it as it is."),
    '#required' => FALSE,
    '#default_value' => $mount_point_windows,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  return $form;
}

function mediamosa_profile_storage_location_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  if (trim($values['current_mount_point']) == '') {
    form_set_error('current_mount_point', t("The current Linux mount point can't be empty."));
  }
  elseif (!is_dir($values['current_mount_point'])) {
    form_set_error('current_mount_point', t('The current Linux mount point is not a directory.'));
  }
  elseif (!is_writable($values['current_mount_point'])) {
    form_set_error('current_mount_point', t('The current Linux mount point is not writeable for the apache user.'));
  }
}

function mediamosa_profile_storage_location_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  variable_set('mediamosa_current_mount_point', $values['current_mount_point']);
  variable_set('mediamosa_current_mount_point_windows', $values['current_mount_point_windows']);

  // Inside the storage location, create a Mediamosa storage structure.
  // data.
  mediamosa_profile_mkdir($values['current_mount_point'] . '/data');
  for ($i = 0; $i <= 9; $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . chr($i));
  }
  // data/stills.
  mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills');
  for ($i = 0; $i <= 9; $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . chr($i));
  }
  // Other.
  mediamosa_profile_mkdir($values['current_mount_point'] . '/data/transcode');
  mediamosa_profile_mkdir($values['current_mount_point'] . '/links');
  mediamosa_profile_mkdir($values['current_mount_point'] . '/download_links');
  mediamosa_profile_mkdir($values['current_mount_point'] . '/still_links');
  mediamosa_profile_mkdir($values['current_mount_point'] . '/ftp');
}

/**
 * Client application.
 * Tasks callback.
 */
function mediamosa_profile_client_application_form() {
  $form = array();

  $form['client_application'] = array(
    '#type' => 'item',
    '#title' => t('Client application'),
    '#description' => t("You can create a client application here following the link: !link. The link opens in a new window. After you created a client application you have to came back here.<br /><br />If you don't want to do it now, you can do it later in Admin / MediaMosa / Configuration / Client application.<br /><br />You don't have to create a client application, if you are going to migrate your old 1.7 MediaMosa data to version 2.", array('!link' => l(t('Create client application'), 'admin/mediamosa/config/app/add', array('attributes' => array('target' => '_blank'))))),
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

/**
 * Configure the server.
 * Tasks callback.
 */
function mediamosa_profile_configure_server($install_state) {
  $output = '';
  $error = FALSE;

  $server_name = mediamosa_profile_server_name();


  // Configure the servers.

  // Mediamosa server table.
  db_query("
    UPDATE {mediamosa_server}
    SET server_uri = REPLACE(server_uri, 'http://localhost', :server)
    WHERE LOCATE('http://localhost', server_uri) > 0", array(
    ':server' => 'http://' . $server_name,
  ));
  db_query("
    UPDATE {mediamosa_server}
    SET uri_upload_progress = REPLACE(uri_upload_progress, 'http://example.org', :server)
    WHERE LOCATE('http://example.org', uri_upload_progress) > 0", array(
    ':server' => 'http://' . $server_name,
  ));

  // Mediamosa node revision table.
  $result = db_query("SELECT nid, vid, revision_data FROM {mediamosa_node_revision}");
  foreach ($result as $record) {
    $revision_data = unserialize($record->revision_data);
    if (isset($revision_data['server_uri'])) {
      $revision_data['server_uri'] = str_replace('mediamosa.local', $server_name, $revision_data['server_uri']);
    }
    if (isset($revision_data['uri_upload_progress'])) {
      $revision_data['uri_upload_progress'] = str_replace('mediamosa.local', $server_name, $revision_data['uri_upload_progress']);
    }
    db_query("
      UPDATE {mediamosa_node_revision}
      SET revision_data = :revision_data
      WHERE nid = :nid AND vid = :vid", array(
      ':revision_data' => serialize($revision_data),
      ':nid' => $record->nid,
      ':vid' => $record->vid,
    ));
  }


  // Configure.
  // URL REST.
  variable_set('mediamosa_cron_url_app', 'http://app.' . $server_name . (substr($server_name, -6) == '.local' ? '' : '.local'));

  // Configure mediamosa connector.
  variable_set('mediamosa_connector_url', 'http://' . $server_name);
  $result = db_query("SELECT app_name, shared_key FROM {mediamosa_app} LIMIT 1");
  foreach ($result as $record) {
    variable_set('mediamosa_connector_username', $record->app_name);
    variable_set('mediamosa_connector_password', $record->shared_key);
  }


  return $error ? $output : NULL;
}

/**
 * Information about cron, apache and migration.
 * Task callback.
 */
function mediamosa_profile_cron_settings_form() {
  $form = array();

  $server_name = mediamosa_profile_server_name();


  // Cron.

  $form['cron'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Cron'),
    '#description' => t('The cron will used to process your uploads and other jobs.'),
  );

  $form['cron']['cron_every_minute'] = array(
    '#type' => 'textarea',
    '#title' => t('Cron every minute'),
    '#attributes' => array('style' => 'font-family: Fixed, monospace;'),
  '#description' => t('You have to copy this content to a file to your home directory: <code>~/bin/cron_every_minute.sh.</code><br />After, you have to modify the file permissions:<br />
    <code>
      chmod a+x ~/bin/cron_every_minute.sh<br />
    </code>'),
    '#default_value' => '#!/bin/sh

#MediaMosa Cron.
/usr/bin/wget -O - -q -t 1 --header="Host: ' . $server_name . '" http://localhost/cron.php?cron_key=' . variable_get('cron_key', ''),
    '#cols' => 60,
    '#rows' => 5,
  );

  $form['cron']['crontab'] = array(
    '#type' => 'textarea',
    '#title' => t('Crontab'),
    '#description' => t('After, you have to modify your crontab: <code>crontab -e</code><br />Add these lines.'),
    '#attributes' => array('style' => 'font-family: Fixed, monospace;'),
    '#default_value' => '# Mediamosa 2
* * * * * ~/bin/cron_every_minute.sh',
    '#cols' => 60,
    '#rows' => 5,
  );

  // Apache.

  $mount_point = variable_get('mediamosa_current_mount_point', '');
  $document_root = mediamosa_profile_document_root();


  $form['apache'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Apache'),
    '#attributes' => array('style' => 'font-family: Fixed, monospace;'),
    '#description' => t("You have to set up your Apache2 following this instruction:<br />
1) Change your site's settings in <code>/etc/apache2/sites-enabled/your-site</code>.<br />
First save your original file, then insert this code to your settings file.<br />
2) Restart your Apache: <code>sudo /etc/init.d/apache2 restart</code><br />") . (strpos($server_name, '/') === FALSE ? '' : t("<b>It is strongly recommended, that you use server name like '<code>@mediamosa</code>', when you install Mediamosa, and not like '<code>@server_name</code>'.</b>", array('@mediamosa' => (substr($server_name, -1) == '/' ? 'mediamosa' : substr($server_name, strrpos($server_name, '/')+1)), '@server_name' => $server_name,))),
  );

  $server_name_clean = substr($server_name, -6) == '.local' ? substr($server_name, 0, strlen($server_name) - 6) : $server_name;

  $form['apache']['apache'] = array(
    '#type' => 'textarea',
    '#title' => t('Apache'),
    '#attributes' => array('style' => 'font-family: Fixed, monospace;'),
    '#default_value' => "<VirtualHost *:80>
    Servername $server_name_clean.local
    ServerAlias beheer.$server_name_clean.local www.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/$server_name_clean\_error.log
    CustomLog /var/log/apache2/$server_name_clean\_access.log combined
    ServerSignature On

    Alias /server-status $document_root
    <Directory $document_root/serverstatus>
        SetHandler server-status
        Order deny,allow
        Deny from all
        Allow from 127.0.0.1
     </Directory>

    # ticket
    Alias /ticket $mount_point/links
    <Directory $mount_point/links>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
    </Directory>
</VirtualHost>

<VirtualHost *:80>
    Servername app1.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app1.$server_name_clean\_error.log
    CustomLog /var/log/apache2/app1.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    Servername app2.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app2.$server_name_clean\_error.log
    CustomLog /var/log/apache2/app2.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    Servername upload.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    <IfModule mod_php5.c>
        php_admin_value post_max_size 2008M
        php_admin_value upload_max_filesize 2000M
        php_admin_value memory_limit 64M
    </IfModule>

    ErrorLog /var/log/apache2/upload.$server_name_clean\_error.log
    CustomLog /var/log/apache2/upload.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    Servername download.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/download.$server_name_clean\_error.log
    CustomLog /var/log/apache2/download.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    Servername job1.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job1.$server_name_clean\_error.log
    CustomLog /var/log/apache2/job1.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    Servername job2.$server_name_clean.local
    ServerAdmin webmaster@localhost
    DocumentRoot $document_root
    <Directory $document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job2.$server_name_clean\_error.log
    CustomLog /var/log/apache2/job2.$server_name_clean\_access.log combined
    ServerSignature On
</VirtualHost>",
    '#cols' => 60,
    '#rows' => 40,
  );


  // Migration.

  $form['migration'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Migration'),
    '#description' => t("You can migrate your data from your old 1.7 Mediamosa installation following these steps:<br />1) Open your <code>settings.mediamosa.php</code> in your <code>sites</code> directory.<br />2) Insert there the following code there and save it.<br />3) Be sure, that the MySQL user of current installation, and the MySQL user of the old databases can read from the other database(s)."),
  );

  $form['migration']['settings'] = array(
    '#type' => 'textarea',
    '#title' => t('Migration'),
    '#attributes' => array('style' => 'font-family: Fixed, monospace;'),
    '#default_value' => "\$databases['mig_memo']['default'] = array(
  'driver' => 'mysql',
  'database' => 'your_old_database',
  'username' => 'your_user_name',
  'password' => 'your_password',
  'host' => 'localhost'
);
\$databases['mig_memo_data']['default'] = array(
  'driver' => 'mysql',
  'database' => 'your_old_database_data',
  'username' => 'your_user_name',
  'password' => 'your_password',
  'host' => 'localhost'
);",
    '#cols' => 60,
    '#rows' => 15,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

/**
 * Advanced mkdir().
 * Check if the directory is exist, before makes it.
 * @param string $check_dir Directory to check.
 */
function mediamosa_profile_mkdir($check_dir) {
  if (!is_dir($check_dir)) {
    mkdir($check_dir);
  }
}

/**
 * Give back the server name.
 */
function mediamosa_profile_server_name() {
  $server_name = url('', array('absolute' => TRUE));
  $server_name = drupal_substr($server_name, 0, -1);
  $server_name = drupal_substr($server_name, drupal_strlen('http://'));
  $server_name = check_plain($server_name);
  return $server_name;
}

/**
 * Give back the document root for install.php.
 */
function mediamosa_profile_document_root() {
  // Document root.
  $script_filename = getenv('PATH_TRANSLATED');
  if (empty($script_filename)) {
    $script_filename = getenv('SCRIPT_FILENAME');
  }
  $script_filename = str_replace('', '/', $script_filename);
  $script_filename = str_replace('//', '/', $script_filename);
  $document_root = dirname($script_filename);
  return $document_root;
}
