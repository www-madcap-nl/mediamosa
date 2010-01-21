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
  * Browser of REST calls.
  */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_restcall_form() {

  // Call the REST call hook to get the registered REST calls.
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  ksort($rest_calls);

  $item_limit = 12;
  $item_count_total = count($rest_calls);
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  $header = array(
    array('data' => t('URI')),
    array('data' => t('Method')),
    array('data' => t('Status')),
    array('data' => t('Version')),
  );

  $form = $rows = array();

  $pos = 0;
  foreach ($rest_calls as $uri => $rest_calls_2) {
    if ($pos < $offset) {
      $pos++;
      continue;
    }
    elseif ($pos >= ($offset + $item_limit)) {
      break;
    }

    $pos++;

    foreach ($rest_calls_2 as $method => $rest_call) {
      $rows[] = array(
        l('/' . $uri, 'admin/mediamosa/browse/restcall/' . str_replace('/', '-', $uri) . '/' . urlencode($method)),
        $method,
        $rest_call[mediamosa_rest_call::STATUS],
        $rest_call[mediamosa_rest_call::VERSION],
      );
    }
  }

  $form['pager_top'] = array(
    '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
  );

  $form['results'] = array(
    '#markup' => theme('table', $header, $rows),
  );

  $form['pager_bottom'] = array(
    '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
  );

  return $form;
}

/**
 * View specific details of a REST call.
 */
function mediamosa_maintenance_browse_restcall_details($uri, $method) {

  $uri = str_replace('-', '/', $uri);
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');

  if (!isset($rest_calls[$uri][$method])) {
    return t('Rest Call not found.');
  }

  $rest_calls[$uri][$method][mediamosa_rest_call::URI] = $uri;
  $rest_calls[$uri][$method][mediamosa_rest_call::METHOD] = $method;

  // Create class from it.
  $o_rest_call = new $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME](mediamosa_rest::set_default_rest_call($rest_calls[$uri][$method]));

  // Turn off so we can call the get_var_setup without problems.
  // Although get_var_setup should not test variables.
  $o_rest_call->set_check_for_unspecified(FALSE);

  // Get the var setup for this call.
  $a_var_setup = $o_rest_call->get_var_setup();

  $header = array(
    array('data' => t('Parameter')),
    array('data' => t('Type')),
    array('data' => t('Description')),
    array('data' => t('Required')),
    array('data' => t('Max. length')),
  );

  if (!isset($a_var_setup[mediamosa_rest_call::VARS])) {
    $a_var_setup[mediamosa_rest_call::VARS] = array();
  }

  // Sort on param.
  ksort($a_var_setup[mediamosa_rest_call::VARS]);
  $form = $rows = array();

  drupal_set_title(
    t('Viewing details rest call URI @uri, method [@method]',
      array(
      '@uri' => '/' . $uri,
      '@method' => $method
      )
    )
  );

  // First required.
  foreach ($a_var_setup[mediamosa_rest_call::VARS] as $param => $a_item) {
    if (isset($a_item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $a_item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      $rows[] = array(
        $param,
        $a_item[mediamosa_rest_call::VAR_TYPE],
        $a_item[mediamosa_rest_call::VAR_DESCRIPTION],
        t('Required'),
        '-',
      );
    }
  }

  // Not required.
  foreach ($a_var_setup[mediamosa_rest_call::VARS] as $param => $a_item) {
    if (!isset($a_item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $a_item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      $rows[] = array(
        $param,
        $a_item[mediamosa_rest_call::VAR_TYPE],
        $a_item[mediamosa_rest_call::VAR_DESCRIPTION],
        t('Optional'),
        '-',
      );
    }
  }

  $contents = theme('table', $header, $rows);

  return $contents;
}