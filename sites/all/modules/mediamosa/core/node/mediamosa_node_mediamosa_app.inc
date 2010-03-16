<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * The node type client application.
 * hook_load / -update / -delete / -insert / -view / -form / -validate hooks for
 * node type mediamosa_app.
 */

/**
 * Static functions used by node type mediamosa_app.
 *
 */
class mediamosa_node_mediamosa_app {
  /**
   * Retrieve the transcoding profiles for dropdown list.
   *
   * @return array
   *  An array with all the profile Ids
   */
  static public function app_get_transcode_profiles() {
    $result = mediamosa_transcode_profile::get_all(array(mediamosa_transcode_profile_db::ID, mediamosa_transcode_profile_db::PROFILE));

    $output = array();
    foreach ($result as $transcode_profile) {
      $output[$transcode_profile['transcode_profile_id']] = $transcode_profile['transcode_profile_id'] . ' - ' . $transcode_profile['profile'];
    }

    return $output;
  }

  /**
   * Return all available versions used in REST calls.
   *
   * @return array
   *  All version numbers
   */
  static public function app_get_all_versions() {

    // Find all REST calls of enabled modules.
    $rest_calls = module_invoke_all('mediamosa_register_rest_call');

    // The REST calls versions.
    $versions = array();

    // Collect all versions in the restcalls.
    foreach ($rest_calls as $rest_call) {
      foreach ($rest_call as $method => $rest_call_info) {
        $versions[$rest_call_info[mediamosa_rest_call::VERSION]] = $rest_call_info[mediamosa_rest_call::VERSION];
      }
    }

    // Must be first.
    $versions['LATEST'] = t('Latest');

    // Sort on version.
    krsort($versions);

    return $versions;
  }
}

/**
 * Insert a random app shared key in the text block.
 */
function _ajax_app_generate_shared_key() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  $commands = array();

  // Build and render the new select element, then return it in JSON format.
  $form_state = array();
  $form = form_builder($form['form_id']['#value'], $form, $form_state);
  $form['client_application']['shared_key']['#value'] = mediamosa_db::uuid(rand(1, 9999));

  $commands[] = ajax_command_replace(NULL, drupal_render($form['client_application']['shared_key']));

  return array('#type' => 'ajax_commands', '#ajax_commands' => $commands);
}

/**
 * Implementation of hook_load().
 */
function mediamosa_app_load(&$nodes) {
  return mediamosa_node_revision::load($nodes);
}

/**
 * Implementation of hook_update().
 */
function mediamosa_app_update($node) {

  if ($node->revision) {
    mediamosa_app_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_app_db::get_fields();

    // Exception is title.
    $node->name = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_app_db::TABLE_NAME, $names);
  }
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_app_insert($node) {

  // Get the field names of the table.
  $names = mediamosa_app_db::get_fields();

  // Exception is title.
  $node->name = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_app_db::TABLE_NAME, $names);
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_app_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_app_db::TABLE_NAME);
}

/**
 * Implementation of hook_view().
 */
function mediamosa_app_view($node, $view_mode = 'full') {

  // Copy name to title.
  $node->title = $node->{mediamosa_app_db::APP_NAME};

  // Default parameters
  $rows = array(
    array(t('Name'), check_plain($node->{mediamosa_app_db::APP_NAME})),
  );

  // Add timestamps
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['app'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implementation of hook_form().
 */
function mediamosa_app_form($node, $form_state) {
  global $user;

  $form = array();

  $form['client_application'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Client application'),
    '#description' => t('Settings for client application.')
  );

  $form['client_application'][mediamosa_app_db::APP_NAME] = array(
    '#type' => 'textfield',
    '#title' => t('Client application name'),
    '#description' => t('Full name for client application.'),
    '#required' => TRUE,
  );

  $form['client_application'][mediamosa_app_db::APP_ID] = array(
    '#type' => 'textfield',
    '#title' => t('App ID'),
    '#description' => t('Unique application ID.'),
    '#required' => TRUE,
  );

  $form['client_application']['quota'] = array(
    '#type' => 'textfield',
    '#title' => t('Quota'),
    '#required' => TRUE,
    '#description' => t('Quota for disk space usage in GB.'),
  );

  $form['client_application']['shared_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Shared key'),
    '#required' => TRUE,
    '#prefix' => '<div id="replace_shared_key_div">',
    '#suffix' => '</div>',
  );

  $form['client_application']['generate_shared_key'] = array(
    '#type' => 'button',
    '#value' => t('Paste generated key'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'replace_shared_key_div',
      'path' => 'admin/mediamosa/config/app/js/form',
    ),
  );

  $form['client_application']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Description of client application'),
    '#required' => FALSE,
  );

  $form['client_application']['active'] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#description' => t('Availability of client application.'),
    '#options' => array(1 => t('Enabled'), 0 => t('Disabled')),
    '#default_value' => 1,
  );

  $form['masterslave'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Master/slave settings'),
    '#description' => t('Control how other clientapplications can access this application')
  );

  $form['masterslave']['allow_masterslave'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow master/slave access'),
    '#description' => t('Allow other client applications to create slave or master assets in this application.'),
    '#default_value' => 'FALSE',
  );

  $form['oai'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('OAI Interface settings'),
    '#description' => t('')
  );

  $form['oai']['is_metadata_public'] = array(
    '#type' => 'select',
    '#title' => t('OAI visibility metadata'),
    '#description' => t('Is metadata of this client application public visible in OAI?'),
    '#options' => array('TRUE' => t('Public'), 'FALSE' => t('Hidden')),
    '#default_value' => 'TRUE',
  );
  $form['oai']['show_external_assets'] = array(
    '#type' => 'select',
    '#title' => t('OAI external references'),
    '#description' => t('Show external references in OAI (in field referenceId)?'),
    '#options' => array('TRUE' => t('show'), 'FALSE' => t('hide')),
    '#default_value' => 'TRUE',
  );

  $form['transcoding'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Transcoding'),
    '#description' => t('Transcoding settings for this client application.')
  );

  $transcoding_pos = array();
  for ($i = -10; $i <= 10; $i++) {
    $transcoding_pos[$i] = $i;
  }

  $form['transcoding']['transcoding_position_from'] = array(
    '#type' => 'select',
    '#title' => t('The transcoding position from'),
    '#options' => $transcoding_pos,
    '#description' => t('Give server priority.'),
  );

  $form['transcoding']['transcoding_position_to'] = array(
    '#type' => 'select',
    '#title' => t('The transcoding position to'),
    '#options' => $transcoding_pos,
    '#description' => t('Give server priority.'),
  );

  $form['transcoding']['transcoding_max_slots'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum transcoding slots allowed'),
    '#default_value' => mediamosa_settings::JOB_MAX_SLOTS,
    '#description' => t('Set the maximum number of transcoding jobs allowed for this application.'),
  );

  $form['transcoding']['always_hint_mp4'] = array(
    '#type' => 'select',
    '#title' => t('Always hint (make streamable) mp4 files'),
    '#options' => array('FALSE' => t('Never'), 'TRUE' => t('Always')),
    '#default_value' => 'TRUE',
  );

  $form['transcoding']['always_insert_md'] = array(
    '#type' => 'select',
    '#title' => t('Always insert extra metadata flv files'),
    '#options' => array('FALSE' => t('Never'), 'TRUE' => t('Always')),
    '#default_value' => 'TRUE',
  );

  $form['transcoding']['preview_profile_id'] = array(
    '#type' => 'select',
    '#title' => t('Preview transcode profile id'),
    '#description' => t('Transcoding profile used for previewing.'),
    '#options' => mediamosa_node_mediamosa_app::app_get_transcode_profiles(),
    '#required' => FALSE,
  );

  $form['client_application']['active_version'] = array(
    '#type' => 'select',
    '#title' => t('Select active version'),
    '#description' => t('Version of the rest calls to execute.'),
    '#options' => mediamosa_node_mediamosa_app::app_get_all_versions(),
    '#default_value' => 'LATEST',
  );

  $form['urls'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('URLs'),
    '#description' => t('URLs shown in the asset settings.')
  );

  $form['urls']['download_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Download url'),
    '#description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}'),
    '#required' => FALSE,
  );
  $form['urls']['stream_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Stream url'),
    '#description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}.'),
    '#required' => FALSE,
  );
  $form['urls']['play_proxy_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Play proxy url'),
    '#description' => t('Shown in the result of rest calls and shown by OAI. Available tags: {asset_id}, {mediafile_id}, {mediafile_filename} and {preview_profile_id}.'),
    '#required' => FALSE,
  );
  $form['urls']['view_asset_url'] = array(
    '#type' => 'textfield',
    '#title' => t('View asset url'),
    '#description' => t('Shown by OAI, field \'Resource Identifier\'. Available tags: {asset_id}.'),
    '#required' => FALSE,
  );
  $form['urls']['still_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Still url'),
    '#description' => t('Shown by OAI for still images. Available tags: {asset_id}.'),
    '#required' => FALSE,
  );

  $owner = array('' => t('none'), 'surfnet' => 'Surfnet', 'kennisnet' => 'Kennisnet');
  if ($user->uid != 1) { // admin
    $unset = (in_array('kennisnet', (array)$user->roles)) ? 'surfnet' : 'kennisnet';
    unset($owner[$unset]);
  }

  $form['client_application']['owner'] = array(
    '#type' => 'select',
    '#title' => t('Owner'),
    '#description' => t('Owner of client application'),
    '#options' => $owner,
  );

  $form['webservice'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Webservices'),
    '#description' => t('Availibility per web service for this client application.')
  );

  // Dynamicly add fields for webservice handles
  if (!empty($node->app_id)) {
    $result = mediamosa_webservice::get_all($node->app_id);
  }
  else {
    $result = mediamosa_webservice::get_all();
  }

  foreach ($result as $row) {
    $form['webservice']['webservice_'. $row['handle']] = array(
      '#type' => 'select',
      '#title' => $row['handle'],
      '#description' => t('Availibility of web service \''. $row['handle'] .'\' for this client application.'),
      '#default_value' => (isset($row->status)) ? drupal_strtoupper($row['status']) : 'TRUE',
      '#options' => array('TRUE' => t('Enabled'), 'FALSE' => t('Disabled')),
    );
  }

  $form['technical'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Technical Contact'),
    '#description' => t('Technical contact person for this client application.')
  );

  $form['technical']['technical_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of technical contact person for this client application.')
  );

  $form['technical']['technical_phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#description' => t('Phone number of technical contact person for this client application.')
  );

  $form['technical']['technical_email'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail'),
    '#description' => t('Email-address of technical contact person for this client application.')
  );

  $form['administrative'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Administrative Contact'),
    '#description' => t('Administrative contact person for this client application.')
  );

  $form['administrative']['administrative_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of administrative contact person for this client application.')
  );

  $form['administrative']['administrative_phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#description' => t('Phone number of administrative contact person for this client application.')
  );

  $form['administrative']['administrative_email'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail'),
    '#description' => t('Email-address of administrative contact person for this client application.'),
    '#required' => FALSE,
  );

  $form['#redirect'] = 'admin/mediamosa/config/app';

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function mediamosa_app_validate(&$node) {

  // We need to know if its an update.
  $is_insert = empty($node->nid);

  // App Id checks.
  if (!is_numeric($node->{mediamosa_app_db::APP_ID})) {
    form_set_error(mediamosa_app_db::APP_ID, t('The Application ID must be between 1 and 9999'));
  }
  else {
    // Get all apps to know if app id is already taken.
    if ($is_insert && mediamosa_app::get_by_appid($node->{mediamosa_app_db::APP_ID})) {
      form_set_error(mediamosa_app_db::APP_ID, t('The Application ID is already taken, please choose another Application ID.'));
    }

    // If we change the app ID, we need to know if its already taken.
    if (!$is_insert) {
      $org_node = node_load($node->nid);
      if ($org_node->{mediamosa_app_db::APP_ID} != $node->{mediamosa_app_db::APP_ID}) {
        if (mediamosa_app::get_by_appid($node->{mediamosa_app_db::APP_ID})) {
          form_set_error(mediamosa_app_db::APP_ID, t('The Application ID is already taken, please choose another Application ID.'));
        }
      }
    }
  }

  // Transcoding position.
  if ($node->{mediamosa_app_db::TRANSCODING_POSITION_FROM} > $node->{mediamosa_app_db::TRANSCODING_POSITION_TO}) {
    form_set_error(mediamosa_app_db::TRANSCODING_POSITION_FROM, t('Transcoding position FROM value is bigger than transcoding position TO.'));
  }

  // Quota.
  if (!is_numeric($node->{mediamosa_app_db::QUOTA})) {
    form_set_error(mediamosa_app_db::QUOTA, t('Please enter a numerical value.'));
  }
}
