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
 * Implementation of hook_install_tasks().
 */
function mediamosa_profile_install_tasks() {
  $tasks = array(
    'mediamosa_profile_storage_location_form' => array(
      'display_name' => st('Storage location'),
      'type' => 'form',
      'run' => variable_get('mediamosa_current_mount_point', '') ? INSTALL_TASK_SKIP : INSTALL_TASK_RUN_IF_NOT_COMPLETED,
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
      'display_name' => 'PHP settings',
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

  $required_extensions = array('bcmath', 'gd', 'mcrypt', 'curl', 'mysql', 'mysqli', 'SimpleXML');
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
  $last_line = exec('ffmpeg 2>&1');
  $output .= '<li>FFmpeg</li>';
  if ($last_line) {
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


  return $error ? $output : NULL;
}

/**
 * Task callback.
 */
function mediamosa_profile_storage_location_form() {
  $form = array();


  return $form;
}

/**
 * Task callback.
 */
function mediamosa_profile_cron_settings_form() {
  $form = array();


  return $form;
}
