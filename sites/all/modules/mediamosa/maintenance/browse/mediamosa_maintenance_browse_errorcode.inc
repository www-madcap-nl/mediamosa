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
 * Browse the error codes.
 */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_errorcode_form() {

  $header = array(
    array('data' => t('ID'), 'field' => mediamosa_error_db::ID),
    array('data' => t('Name'), 'field' => mediamosa_error_db::NAME),
    array('data' => t('Code'), 'field' => mediamosa_error_db::CODE),
    array('data' => t('Operations')),
  );

  $query = mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')->extend('PagerDefault')->extend('TableSort');
  $query->fields('e');
  $query->orderByHeader($header);
  $query->limit(mediamosa_settings::maintenance_items_per_page());
  $result = $query->execute()->fetchAll();

  $form = $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  // Build the rows.
  foreach ($result as $errorcode) {
    $actions = array(
      l(t('Edit'), 'admin/mediamosa/browse/errorcode/'. $errorcode[mediamosa_error_db::ID] . '/edit', $l_options),
    );

    $rows[] = array('data' =>
      array(
        // Cells
        $errorcode[mediamosa_error_db::ID],
        check_plain($errorcode[mediamosa_error_db::NAME]),
        check_plain($errorcode[mediamosa_error_db::CODE]),
        implode(' | ', $actions)
      ),
    );
  }

  $form['log_pager_top'] = array('#theme' => 'pager');
  $form['log_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );
  $form['log_pager_bottom'] = array('#theme' => 'pager');

  return $form;
}

/**
 * The edit form.
 */
function mediamosa_maintenance_browse_errorcode_edit_form($id) {
  exit('foo');
}


