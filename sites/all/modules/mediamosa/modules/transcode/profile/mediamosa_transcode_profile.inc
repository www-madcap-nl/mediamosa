<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
  $result = $query
    ->fields('tp')
    ->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($result as $row) {
    $actions = array(
      l(t('Edit'), 'admin/mediamosa/config/transcode_profile/' . $row[mediamosa_transcode_profile_db::NID] . '/edit', $l_options),
      l(t('Delete'), 'admin/mediamosa/config/transcode_profile/' . $row[mediamosa_transcode_profile_db::NID] . '/delete', $l_options),
    );

    $owner = isset($apps[$row[mediamosa_transcode_profile_db::APP_ID]]) ? $apps[$row[mediamosa_transcode_profile_db::APP_ID]] : t('Unknown owner');
    $rows[] = array(
      'title' => array('data' => l($row[mediamosa_transcode_profile_db::PROFILE], 'admin/mediamosa/config/transcode_profile/' . $row[mediamosa_transcode_profile_db::NID]), 'field' => $row[mediamosa_transcode_profile_db::PROFILE]),
      'description' => $row[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE],
      'tool' => $row[mediamosa_transcode_profile_db::TOOL],
      'owner' => $owner,
      implode(' | ', $actions),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no transcode profiles. Click on the <em>Add</em> tab to create one.')));
  $output .= theme('pager');
  return $output;
}
