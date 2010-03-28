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
 * Statistics latest mediafiles.
 */

// Include default.
module_load_include('inc', 'mediamosa_statistics', 'mediamosa_statistics');

/**
 * List the latest mediafiles.
 */
function mediamosa_statistics_mediafile_latest_form() {

  $build = array();

  $build['mediamosa_statistics_filter'] = drupal_get_form('mediamosa_statistics_filter_form', MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_LATEST);
  $build['mediamosa_statistics_list'] = drupal_get_form('mediamosa_statistics_mediafile_latest_list_form');

  return $build;
}

function mediamosa_statistics_mediafile_latest_list_form() {

  // Get session.
  $session = isset($_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_LATEST]) ? $_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_LATEST] : array();

  // Get app owners.
  $app_owners = mediamosa_app::collect_app_owners();

  $app_ids = array();
  if (!empty($session['app_id'])) {
    $app_ids[] = $session['app_id'];
  }
  if (!empty($session['app_owner']) && isset($app_owners[$session['app_owner']])) {
    $app_ids += $app_owners[$session['app_owner']];
  }

  $params = array(
    'app_id' => $app_ids,
  );

  // Do the rest call.
  $result = mediamosa_response_connector::static_do_restcall_drupal('statistics/newestmediafiles', 'GET', $params);

  // Our header.
  $header = array(
    array('data' => t('App ID')),
    array('data' => t('Filename')),
    array('data' => t('Owner ID')),
    array('data' => t('Created')),
  );

  $rows = array();

  foreach ($result['items'] as $row) {
    $title = empty($row['filename']) ? $row['mediafile_id'] : $row['filename'];

    $rows[] = array(
      $row['app_id'],
      mediamosa_lib::l_asset($row['asset_id'], $title),
      check_plain($row[mediamosa_asset_mediafile_db::OWNER_ID]),
      $row[mediamosa_asset_mediafile_db::CREATED],
    );
  }

  //$form['pager_top'] = array('#theme' => 'pager');
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No mediafiles were found.')
  );
  //$form['pager_bottom'] = array('#theme' => 'pager');

  return $form;
}
