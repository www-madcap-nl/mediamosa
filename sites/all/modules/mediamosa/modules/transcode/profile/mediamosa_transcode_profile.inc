<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
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
 * Implementation of hook_view().
 */
function mediamosa_transcode_profile_view($node, $build_mode = 'full') {
  // Default parameters
  $rows = array(
    array(t('Title'), check_plain($node->title)),
    array(t('App ID'), $node->app_id),
    array(t('Tool'), $node->tool),
    array(t('File extension'), $node->file_extension),
    array(t('Is default profile'), t(drupal_ucfirst(strtolower($node->is_default_profile)))),
  );

  // Optional parameters
  $all_options = mediamosa_server_transcoding::get_transcode_tool_mappings($node->tool);
  foreach ($node->options as $key => $value) {
    $rows[] = array($all_options[$key]['field_title'], check_plain($value));
  }

  // Add timestamps
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['transcode_profile'] = array(
    '#markup' => theme('table', array(t('Parameter'), t('Value')), $rows),
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

  // Gather all apps
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::NAME), mediamosa_app_db::NAME, 'ASC') as $app) {
    $all_apps[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::NAME];
  }

  $form['required']['app_id'] = array(
    '#title' => t('Application'),
    '#type' => 'select',
    '#default_value' => $node->{mediamosa_transcode_profile_db::APP_ID},
    '#required' => TRUE,
    '#options' => $all_apps,
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
    $options = mediamosa_server_transcoding::get_transcode_tool_mappings($node->tool);

    $form['tool_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('@tool optional parameters', array('@tool' => drupal_ucfirst($node->tool))),
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
    );

    foreach ($options as $option) {
      $form['tool_options']['options_'. $option['tmid']] = array(
        '#type' => $option['field_type'],
        '#description' => $option['field_description'],
        '#title' => $option['field_title'],
        '#required' => ($option['required'] == 'TRUE'),
        '#default_value' => (isset($node->options[$option['tmid']])) ? $node->options[$option['tmid']] : '',
      );

      if ($option['field_type'] == 'select') {
        $form['tool_options']['options_'. $option['tmid']]['#options'] = drupal_map_assoc(array_merge(
          array('' => ''),
          explode(';', $option['allowed_value'])
        ));
      }
    }
  }

  return $form;
}
