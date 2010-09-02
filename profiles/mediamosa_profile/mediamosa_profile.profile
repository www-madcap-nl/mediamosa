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
define('MEDIAMOSA_PROFILE_TEST_LUA_LPEG', 'lua works');

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
 * Retrieve the version.
 */
function _mediamosa_profile_get_version() {
  $inc = include_once (DRUPAL_ROOT . '/sites/all/modules/mediamosa/mediamosa.version.inc');

  // Try to include the settings file.
  $version = $inc ? mediamosa_version::get_current_version_str(TRUE) : null;

  return ($version ? $version : '');
}

/**
 * Set up title.
 */
function _mediamosa_profile_get_title() {
  return 'Installing MediaMosa ' . _mediamosa_profile_get_version();
}

/**
 * Implementation of hook_install_tasks().
 */
function mediamosa_profile_install_tasks() {

  drupal_set_title(_mediamosa_profile_get_title());

  $tasks = array(
    'mediamosa_profile_php_settings' => array(
      'display_name' => st('Verify PHP settings'),
      'run' => 'INSTALL_TASK_RUN_IF_REACHED',
    ),
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
    ),
    'mediamosa_profile_apache_settings_form' => array(
      'display_name' => st('Apache settings'),
      'type' => 'form',
    ),
    'mediamosa_profile_migration_form' => array(
      'display_name' => st('Migration of your curent database'),
      'type' => 'form',
    ),
    'mediamosa_profile_cron_settings_form' => array(
      'display_name' => st('Cron settings'),
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
    'install_select_profile' => array(
      'display_name' => st('Choose profile'),
      'display' => count($install_state['profiles']) != 1,
      'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),
    'install_select_locale' => array(
      'display_name' => st('Choose language'),
      'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),
    'install_load_profile' => array(
      'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),
    'mediamosa_profile_intro_form' => array(
      'display_name' => st('Welcome to MediaMosa'),
      'type' => 'form',
    ),
    'install_verify_requirements' => array(
      'display_name' => st('Verify requirements'),
    ),
  ) + $tasks;
}

/**
 * The profile intro.
 *
 * @param unknown_type $install_state
 */
function mediamosa_profile_intro_form($form, &$form_state, &$install_state) {

  $form['intro'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Welcome to MediaMosa'),
  );

  $form['intro']['welcome'] = array(
    '#markup' => t("<p>MediaMosa is a full featured, webservice oriented media management
      and distribution platform. MediaMosa is implemented as a modular
      extension to the open source Drupal system. You'll find the MediaMosa
      specific modules in sites/all/modules, the remainder of the tree is
      the standard Drupal core.
    </p>
    <p>
    Before procceding, please viits our !quick_install.
    </p>
    <p>
      Please visit !mediamosa.org for more information, announcements,
      documentation, community forums, and new releases.
    </p>
    <p>
    References:
    <ul>
      <li>!mediamosa.org</a></li>
      <li><a href=\"http://www.drupal.org\">http://www.drupal.org</a></li>
    </ul>
  </p>",
    array(
      '!mediamosa.org' => l('http://www.mediamosa.org', 'http://www.mediamosa.org/', array('absolute' => TRUE)),
      '!quick_install' => l('quick install page', 'http://mediamosa.org/trac/wiki/Quick%20install', array('absolute' => TRUE))
    ))
  );

  $form['database'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('MediaMosa database setup'),
  );

  $form['database']['Message'] = array(
    '#markup' => t("<p>
    We advice using !mysql v5.1, or use MySQL variant like !mariadb.
    MediaMosa is currently <b>untested</b> with !postgresql.
   </p>
   <p>Use the database <b>mediamosa</b> example below to create your database 'mediamosa' with user 'memo' before proceeding.</p>
    <code>
        # The 'yourpasswd' entries below needs to be a password you specify.<br />
        <br />
        # Create the database first.<br />
        CREATE DATABASE mediamosa DEFAULT CHARSET=utf8;<br />
        <br />
        # You may choose to specify a source host instead of '%'.<br />
        CREATE USER 'memo'@'%' IDENTIFIED BY 'yourpasswd';<br />
        <br />
        # Create the localhost access for user 'memo'.<br />
        CREATE USER 'memo'@'localhost' IDENTIFIED BY 'yourpasswd';<br />
        <br />
        # Now grant usage for user 'memo' on the 'mediamosa' database.<br />
        GRANT USAGE ON mediamosa.* TO 'memo'@'%' IDENTIFIED BY 'yourpasswd' WITH MAX_QUERIES_PER_HOUR 0 MAX_CONNECTIONS_PER_HOUR 0 MAX_UPDATES_PER_HOUR 0 MAX_USER_CONNECTIONS 0;<br />
        <br />
        GRANT ALL ON mediamosa.* TO 'memo'@'%';<br />
    </code>
    <p>
        You may change the 'mediamosa' database prefix and the database user name.<br />
        <br />
        If you want to migrate your current MediaMosa v1.7 database to the new 2.x version, you have to create or have a database user, which has enough rights to read your current v1.7 databases.</p>
   ", array(
    '!mysql' => l('MySQL', 'http://mysql.com/'),
    '!mariadb' => l('MariaDB', 'http://mariadb.org/'),
    '!postgresql' => l('PostgreSQL', 'http://www.postgresql.org/')
   ))
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );
  $form['errors'] = array();

  return $form;
}

function mediamosa_profile_intro_form_validate($form, &$form_state) {
}

/**
 * Submit the intro form.
 */
function mediamosa_profile_intro_form_submit($form, &$form_state) {
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

  $required_extensions = array('bcmath', 'gd', 'curl', 'mysql', 'mysqli', 'SimpleXML');
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
    drupal_set_message(t('Lua is not found. You need to install it first. You can find more information how to install LUA !here', array('!here' => l('here', 'http://mediamosa.org/forum/viewtopic.php', array('absolute' => TRUE, 'external' => TRUE, 'query' => array('f'=> '13', 't' => '175', 'start' => '10'), 'fragment' => 'p687')))), 'error');
    $error = TRUE;
  }


  // Lpeg.
  $last_line = exec('lua profiles/mediamosa_profile/lua/lua_test 2>&1', $retval);
  $output .= '<li>Lua LPeg</li>';
  if ($last_line != MEDIAMOSA_PROFILE_TEST_LUA_LPEG) {
    drupal_set_message(t('Lpeg  extension of Lua is not found. Please, install it first.'), 'error');
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
    '#title' => t('MediaMosa SAN/NAS Mount point'),
    '#description' => t('The mount point is used to store the mediafiles.<br />Make sure the Apache user has write access to the MediaMosa SAN/NAS mount point.'),
    '#required' => TRUE,
    '#default_value' => $mount_point,
  );

  $form['current_mount_point_windows'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa SAN/NAS Mount point for Windows'),
    '#description' => t("The mount point is used to store the mediafiles.<br />Make sure the webserver has write access to the Windows MediaMosa SAN/NAS mount point.<br />If you don't use Windows, just leave it as it is."),
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

  // Inside the storage location, create a MediaMosa storage structure.
  // data.
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/data');
  for ($i = 0; $i <= 9; $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/' . chr($i));
  }
  // data/stills.
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills');
  for ($i = 0; $i <= 9; $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . $i);
  }
  for ($i = ord('a'); $i <= ord('z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . chr($i));
  }
  for ($i = ord('A'); $i <= ord('Z'); $i++) {
    _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/stills/' . chr($i));
  }
  // Other.
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/data/transcode');
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/links');
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/download_links');
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/still_links');
  _mediamosa_profile_mkdir($values['current_mount_point'] . '/ftp');
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

  $server_name = _mediamosa_profile_server_name();


  // Configure the servers.

  // MediaMosa server table.
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

  // MediaMosa node revision table.
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
  variable_set('mediamosa_cron_url_app', 'http://app1.' . $server_name . (substr($server_name, -6) == '.local' ? '' : '.local'));

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

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();

  // Cron.

  $form['cron'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Cron setup'),
    '#description' => t('The cron will be used trigger MediaMosa every minute for background proccess. The setup for cron is required for be able to run MediaMosa.'),
  );

  $form['cron']['cron_every_minute_text_1'] = array(
    '#markup' => t("<h5>Creating the cron script</h5><p>Copy the content below into a file in your home directory:<br /><small>(Note: The 'bin' directory might not exists in your home directory, create when needed.)</small></p><p><code>~/bin/cron_every_minute.sh.</code></p>"),
  );

  $form['cron']['cron_every_minute'] = array(
    '#type' => 'textarea',
    '#attributes' => array('class' => array('mm-profile-textarea')),
    '#default_value' => '#!/bin/sh

# Trigger MediaMosa Cron Page.
/usr/bin/wget -O - -q -t 1 --header="Host: ' . $server_name . '" http://localhost/cron.php?cron_key=' . variable_get('cron_key', ''),
    '#rows' => 6,
  );

  $form['cron']['cron_every_minute_text_2'] = array(
    '#markup' => t("<p>Save the file and modify the file permissions:</p>
    <p><code>chmod a+x ~/bin/cron_mediamosa.sh</code></p>"),
  );

  $form['cron']['crontab_text'] = array(
    '#markup' => t('<h5>Adding our new script to the crontab</h5>Modify your cron using crontab, this will run our script every minute:<p><code>crontab -e</code></p><p>Add this line at the bottom:</p>'),
  );

  $form['cron']['crontab'] = array(
    '#type' => 'textarea',
    '#attributes' => array('class' => array('mm-profile-textarea')),
    '#default_value' => '* * * * * ~/bin/cron_mediamosa.sh',
    '#rows' => 2,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

/**
 * Information about cron, apache and migration.
 * Task callback.
 */
function mediamosa_profile_apache_settings_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();



  // Apache.

  $mount_point = variable_get('mediamosa_current_mount_point', '');
  $document_root = _mediamosa_profile_document_root();


  $form['apache'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Apache'),
    '#description' => t("You have to set up your Apache2 following this instruction"),
  );

  $form['apache']['apache_options'] = array(
    '#markup' => t("<ol><li>Change your site's settings in <code>/etc/apache2/sites-enabled/your-site</code>.<br />First save your original file, then insert this code to your settings file.</li>
<li>Restart your Apache:<br /><code>sudo /etc/init.d/apache2 restart</code><br />") .
  (strpos($server_name, '/') === FALSE ? '' : t("<b>It is strongly recommended, that you use server name like '<code>@mediamosa</code>', when you install MediaMosa, and not like '<code>@server_name</code>'.</b>", array('@mediamosa' => (substr($server_name, -1) == '/' ? 'mediamosa' : substr($server_name, strrpos($server_name, '/') + 1)), '@server_name' => $server_name))) . '</li></ol>',
  );

  $server_name_clean = substr($server_name, -6) == '.local' ? substr($server_name, 0, strlen($server_name) - 6) : $server_name;

  $form['apache']['apache'] = array(
    '#type' => 'textarea',
    '#title' => t('Apache'),
    '#attributes' => array('class' => array('mm-profile-textarea')),
    '#default_value' => strtr("<VirtualHost *:80>
    ServerName !server_name_clean.local
    ServerAlias admin.!server_name_clean.local www.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/!server_name_clean_error.log
    CustomLog /var/log/apache2/!server_name_clean_access.log combined
    ServerSignature On

    Alias /server-status !document_root
    <Directory !document_root/serverstatus>
        SetHandler server-status
        Order deny,allow
        Deny from all
        Allow from 127.0.0.1
     </Directory>

    # ticket
    Alias /ticket !mount_point/links
    <Directory !mount_point/links>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
    </Directory>
</VirtualHost>

<VirtualHost *:80>
    ServerName app1.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app1.!server_name_clean_error.log
    CustomLog /var/log/apache2/app1.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName app2.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/app2.!server_name_clean_error.log
    CustomLog /var/log/apache2/app2.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName upload.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
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

    ErrorLog /var/log/apache2/upload.!server_name_clean_error.log
    CustomLog /var/log/apache2/upload.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName download.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/download.!server_name_clean_error.log
    CustomLog /var/log/apache2/download.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName job1.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job1.!server_name_clean_error.log
    CustomLog /var/log/apache2/job1.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>

<VirtualHost *:80>
    ServerName job2.!server_name_clean.local
    ServerAdmin webmaster@!server_name_clean.local
    DocumentRoot !document_root
    <Directory !document_root>
        Options FollowSymLinks MultiViews
        AllowOverride All
        Order allow,deny
        allow from all
    </Directory>

    ErrorLog /var/log/apache2/job2.!server_name_clean_error.log
    CustomLog /var/log/apache2/job2.!server_name_clean_access.log combined
    ServerSignature On
</VirtualHost>", array(
      '!server_name_clean' => $server_name_clean,
      '!document_root' => $document_root,
      '!mount_point' => $mount_point,
    )
  ),
    '#cols' => 60,
    '#rows' => 40,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

/**
 * Information about 1.7 -> 2.x migration.
 * Task callback.
 */
function mediamosa_profile_migration_form() {
  $form = array();

  // Add our css.
  drupal_add_css('profiles/mediamosa_profile/mediamosa_profile.css');

  // Get the server name.
  $server_name = _mediamosa_profile_server_name();

  // Migration.

  $form['migration'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Migrating your 1.7.x database to 2.x'),
    '#description' => t("If you already have an MediaMosa 1.x database, then you need to migrate the database to the new 2.x database format. Migrate 1.7.x database from your current 1.7.x MediaMosa installation to 2.x database by following these steps:
    <ol>
      <li>Open the <code>settings.mediamosa.php</code> in your new MediaMosa 2.x installation in the <code>sites</code> directory.</li>
      <li>Insert the content below from the text box and change the settings to match your 1.7.x MySQL setup for the MediaMosa 1.x MySQL user. In the file there is already an commented out version you can edit.</li>
    </ol><p>You can start the migration process once you have completed the installation. To start the migration, go to MediaMosa home, then click on tab 'Configuration'. Click on the link 'MediaMosa 1.7.x migration tool' to open up the migration tool. The migration tool will pre-check before you can start the migration.</p>
    <p><b>Important notes:</b></p>
    <ul>
      <li>Both databases (1.7.x and 2.x) must be on the same MySQL database server. You can not migrate with 2 servers.</li>
      <li>Before you migrate, at least do the migration once for testing before planning the final migration to be sure the migration will be successful.</li>
      <li>All data will be migrated, except for the ticket and jobs tables. The ticket table holds current session for downloading and play tickets of mediafiles. The job tables hold information about running jobs and old jobs. So make sure that your server is no longer running any jobs when you migrate.</li>
      <li>The 1.7.x database will only be used for reading, nothing will change on your 1.7.x database. However, the 2.x database needs to be clean install for migration to be successful.</li>
    </ul>"),
  );

  $form['migration']['settings'] = array(
    '#type' => 'textarea',
    '#title' => t('Migration setup for sites/settings.mediamosa.php'),
    '#attributes' => array('class' => array('mm-profile-textarea')),
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
function _mediamosa_profile_mkdir($check_dir) {
  if (!is_dir($check_dir)) {
    mkdir($check_dir);
  }
}

/**
 * Give back the server name.
 */
function _mediamosa_profile_server_name() {
  $server_name = url('', array('absolute' => TRUE));
  $server_name = drupal_substr($server_name, 0, -1);
  $server_name = drupal_substr($server_name, drupal_strlen('http://'));
  $server_name = check_plain($server_name);
  return $server_name;
}

/**
 * Give back the document root for install.php.
 */
function _mediamosa_profile_document_root() {
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
