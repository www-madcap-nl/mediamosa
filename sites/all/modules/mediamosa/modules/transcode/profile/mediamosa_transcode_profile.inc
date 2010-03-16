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
  * Mediamosa transcode profile function.
  */

/**
 * Collect the client apps as app_id => title.
 */
function _mediamosa_get_apps() {
  // Get the apps for the drop down listing.
  $apps = array(0 => t('Global profile (no owner)'));

  // Gather all apps
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME, 'ASC') as $app) {
    $apps[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::APP_NAME];
  }

  return $apps;
}

/**
 * Show the listing of the current transcode profiles.
 */
function _mediamosa_transcode_profile_list() {

  // Get the apps.
  $apps = _mediamosa_get_apps();

  $header = array(
    array('data' => t('Profile'), 'field' => 'tp.' . mediamosa_transcode_profile_db::PROFILE),
    array('data' => t('Is Default'), 'field' => 'tp.' . mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    array('data' => t('Tool'), 'field' => 'tp.' . mediamosa_transcode_profile_db::TOOL),
    array('data' => t('Owner'), 'field' => 'tp.' . mediamosa_transcode_profile_db::APP_ID),
  );

  $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
  $result = $query
    ->fields('tp')
    ->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute();

  $rows = array();
  foreach ($result as $row) {
    $row = (object)$row;
    $owner = isset($apps[$row->app_id]) ? $apps[$row->app_id] : t('Unknown owner');
    $rows[] = array(
      'title' => array('data' => l($row->profile, 'admin/mediamosa/config/transcode_profile/'. $row->nid), 'field' => $row->profile),
      'description' => $row->is_default_profile,
      'tool' => $row->tool,
      'owner' => $owner,
    );
  }

  $content = theme('table', array('header' => $header, 'rows' => $rows));
  $content .= theme('pager');

  return $content;
}

/**
 * Implementation of hook_view().
 */
function mediamosa_transcode_profile_view($node, $view_mode = 'full') {

  // Get the apps.
  $apps = _mediamosa_get_apps();

  // Default parameters
  $rows = array(
    array(t('Title'), check_plain($node->title)),
    array(t('Application'), isset($apps[$node->app_id]) ? $apps[$node->app_id] : 'Unknown or deleted application'),
    array(t('Tool'), $node->tool),
    array(t('File extension'), $node->file_extension),
    array(t('Is default profile'), t(drupal_ucfirst(strtolower($node->is_default_profile)))),
  );

  // Optional parameters
  $all_options = mediamosa_transcode_profile::get_by_tool(NULL, $node->tool);
  if (!empty($node->options)) {
    foreach ($node->options as $key => $value) {
      $rows[] = array($all_options[$key]['field_title'], check_plain($value));
    }
  }

  // Add timestamps
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['transcode_profile'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implementation of hook_form().
 */
function mediamosa_transcode_profile_form($node, $form_state) {
  // If we're inserting a new node, set some defaults:
  if (!isset($node->nid)) {
    $node->app_id = 0;
    $node->tool = 'ffmpeg';
    $node->is_default_profile = 'FALSE';
    $node->file_extension = '';
  }

  $form = array();

  $form['required'] = array(
    '#type' => 'fieldset',
    '#title' => t('Required parameters'),
    '#description' => t('Required parameters for a transcoding profile.'),
  );

  // Get the apps for the drop down listing.
  $apps = _mediamosa_get_apps();

  $form['required']['app_id'] = array(
    '#title' => t('Application'),
    '#type' => 'select',
    '#default_value' => $node->{mediamosa_transcode_profile_db::APP_ID},
    '#required' => TRUE,
    '#options' => $apps,
    '#description' => t('The application that owns this profile.'),
  );
  $form['required']['title'] = array(
    '#title' => t('Profile name'),
    '#type' => 'textfield',
    '#default_value' => $node->title,
    '#required' => TRUE,
    '#description' => t('The name of this transcode profile.'),
  );
  $form['required']['is_default_profile'] = array(
    '#title' => t('Is this the default profile?'),
    '#type' => 'select',
    '#default_value' => $node->is_default_profile,
    '#options' => array('TRUE' => t('Yes'), 'FALSE' => t('No')),
    '#required' => TRUE,
    '#description' => t('If a transcoding profile is ommited from a transcoding request, the default profile will be choosen.'),
  );
  $form['required']['tool'] = array(
    '#title' => t('Encoder used'),
    '#type' => 'select',
    '#default_value' => $node->tool,
    '#options' => array('ffmpeg' => t('FFMpeg'), 'windows' => t('Windows')),
    '#disabled' => (isset($node->nid)),
    '#description' => t('The encoding tool used to perform the transcoding. Currently there are 2 options available: FFMpeg and Windows (used for WMV).'),
  );
  $form['required']['file_extension'] = array(
    '#title' => t('File extension'),
    '#type' => 'textfield',
    '#default_value' => $node->file_extension,
    '#required' => TRUE,
    '#description' => t('This file extension is only used for the generated filename, which will be the original filename (minus original extension) plus this extension.'),
  );

  if (isset($node->nid)) {
    $a_tools = mediamosa_transcode_profile::get_by_tool(NULL, $node->tool)->fetchAll();

    $form['tool_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('@tool optional parameters', array('@tool' => drupal_ucfirst($node->tool))),
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
    );

    // Show all parameters for profile.
    foreach ($a_tools as $key => $a_tool) {
      $a_tool[mediamosa_transcode_mapping_db::ALLOWED_VALUE] = trim($a_tool[mediamosa_transcode_mapping_db::ALLOWED_VALUE]) == '' ? array() : explode(';', $a_tool[mediamosa_transcode_mapping_db::ALLOWED_VALUE]);

      // Tool required?
      $is_required = $a_tool[mediamosa_transcode_mapping_db::REQUIRED] == mediamosa_transcode_mapping_db::REQUIRED_TRUE;
      $weight = $is_required ? -10 : -15;

      if (empty($a_tool[mediamosa_transcode_mapping_db::ALLOWED_VALUE])) {
        $form['tool_options'][$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]] = array(
          '#description' => t(''),
          '#type' => 'textfield',
          '#title' => t($a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]),
          '#required' => $is_required,
          '#default_value' => isset($node->{$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]}) ? $node->{$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]} : '',
          '#size' => 25,
          '#description' => t('The allowed value is between @min_value and @max_value.', array('@min_value' => $a_tool[mediamosa_transcode_mapping_db::MIN_VALUE], '@max_value' => $a_tool[mediamosa_transcode_mapping_db::MAX_VALUE])),
          '#weight' => $weight
        );
      }
      else {// Select box.
        $a_select_box = array();
        foreach ($a_tool[mediamosa_transcode_mapping_db::ALLOWED_VALUE] as $allowed_value) {
          $a_select_box[$allowed_value] = $allowed_value;
        }

        $form['tool_options'][$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]] = array(
          '#description' => t(''),
          '#type' => 'select',
          '#title' => t($a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]),
          '#required' => $is_required,
          '#default_value' => isset($node->{$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]}) ? $node->{$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]} : '',
          '#options' => $a_select_box,
          '#weight' => $weight['general']
        );
      }
    }
  }

  return $form;
}

/**
 * Implementation of hook_form_alter().
 */
function mediamosa_transcode_form_alter(&$form, &$form_state, $form_id) {

  if ($form_id == 'mediamosa_transcode_profile_node_form') {
    unset($form['body']);
  }
}
