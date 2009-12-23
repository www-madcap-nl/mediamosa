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
  * The configuration module helper functions.
  */

function mediamosa_configuration_watchdog() {
  $content = t('List of the 50 most recent log entries.');

  $levels = watchdog_severity_levels();
  $types = array('job', 'jobhandler', 'jobserver', 'page not found', 'server');
  $result = db_query("SELECT * FROM {watchdog} WHERE type IN ('". implode("', '", $types) ."') ORDER BY timestamp DESC LIMIT 50");

  $rows = array(); // fix notice
  foreach ($result as $row) {
    $rows[] = array($row->type, nl2br($row->message), $levels[$row->severity], format_date($row->timestamp, 'small'));
  }
  $content .= theme('table', array(t('Type'), t('Message'), t('Severity'), t('Date')), $rows);

  return $content;
}

function mediamosa_configuration_transcode_profile_list() {
  require_once('sites/all/modules/mediamosa/core/transcode/profile/mediamosa_transcode_profile_db.inc');

  // Our header defenition
/*
  $header = array(
    array('data' => t('Profile'), 'field' => 'tp.'. mediamosa_transcode_profile_db::PROFILE),
    array('data' => t('IsDefault'), 'field' => 'tp.'. mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    array('data' => t('Tool'), 'field' => 'tp.'. mediamosa_transcode_profile_db::TOOL),
    t('App ID'),
  );
*/
  $header = array(t('Profile'), t('IsDefault'), t('Tool'), t('App ID'));

  $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')->extend('PagerDefault'); // ->extend('TableSort');
  $result = $query
    ->fields('tp')
    ->limit(10)
    ->execute();

  $rows = array();
  foreach ($result as $row) {
    $row = (object)$row;
    $owner = ($row->app_id == 0) ? t('Unknown') : $row->app_id;
    $rows[] = array(
      'title' => array('data' => l($row->profile, 'mm_transcode_profile/'. $row->transcode_profile_id), 'field' => $row->profile),
      'description' => $row->is_default_profile,
      'tool' => $row->tool,
      'owner' => $owner,
    );
  }

  $content = theme('table', $header, $rows);
  $content .= theme('pager');

  return $content;
}

/**
 * Simple profile view table
 */
function mediamosa_configuration_transcode_profile_view($profile_id) {
  require_once('sites/all/modules/mediamosa/core/transcode/profile/mediamosa_transcode_profile_db.inc');

  $profile = mediamosa_db::db_query(
    "SELECT * FROM {#transcode_profile} WHERE #transcode_profile_id = :transcode_profile_id",
    array(
      '#transcode_profile' => mediamosa_transcode_profile_db::TABLE_NAME,
      '#transcode_profile_id' => mediamosa_transcode_profile_db::ID,
      ':transcode_profile_id' => $profile_id,
    ))
    ->fetchObject();

  drupal_set_title($profile->profile);

  $rows = array(
    array(t('Profile name'), $profile->profile),
    array(t('Profile id'), $profile->transcode_profile_id),
    array(t('Default profile'), $profile->is_default_profile),
    array(t('Tool'), $profile->tool),
    array(t('File extension'), $profile->file_extension),
    array(t('Command'), $profile->command),
    array(t('Created'), format_date(strtotime($profile->created), 'small')),
    array(t('Changed'), format_date(strtotime($profile->changed), 'small')),
  );

  $output = theme('table', array(), $rows);
  $output .= l(t('Return'), 'mm_transcode_profile');

  return $output;
}
