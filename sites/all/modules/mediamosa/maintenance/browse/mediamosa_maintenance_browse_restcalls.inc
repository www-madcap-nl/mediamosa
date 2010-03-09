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
  * Browser of REST calls.
  */

/**
 * The main browse form.
 */
function mediamosa_maintenance_browse_restcall_form() {

  // Call the REST call hook to get the registered REST calls.
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  ksort($rest_calls);

  $item_limit = 24;
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
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  if (!isset($rest_calls[$uri][$method])) {
    return t('Rest Call not found.');
  }

  $rest_calls[$uri][$method][mediamosa_rest_call::URI] = $uri;
  $rest_calls[$uri][$method][mediamosa_rest_call::METHOD] = $method;

  $a_rest_call = mediamosa_rest::set_default_rest_call($rest_calls[$uri][$method]);

  // Create class from it.
  $o_rest_call = new $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME]($a_rest_call);

  // Check if override for class selection is here.
  if (method_exists($o_rest_call, 'get_object_rest_call')) {
    $o_rest_call = $o_rest_call->get_object_rest_call($a_rest_call, isset($a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES]) ? $a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES] : array());
  }

  // Set as uri values, so we dont fail checks.
  if (isset($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES])) {
    foreach ($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES] as $param => $value) {
      $o_rest_call->{mediamosa_rest_call::URI_PARAMS}[$param] = $value;
    }
  }

  // Turn off so we can call the get_var_setup without problems.
  // Although get_var_setup should not test variables.
  $o_rest_call->set_check_for_unspecified(FALSE);

  // Get the var setup for this call.
  $a_var_setup = $o_rest_call->get_var_setup();

  drupal_set_title(
    t('Viewing details rest call URI @uri, method [@method]',
      array(
      '@uri' => '/' . $uri,
      '@method' => $method
      )
    )
  );

  // Show the title and description
  $contents = '<h3>' . $o_rest_call->title . '</h3>';

  // @todo: find a better text2html
  $contents .= '<p>' . str_replace("\n\n", "</p>\n<br>\n<p>", $o_rest_call->description) . '</p>';

  // Show rest call url.
  $contents .= '<h3>' . t('Request URL') . '</h3>';
  $contents .= t('@uri [@method]', array('@uri' => '/' . $uri, '@method' => $method)) . '</p>';

  // Show rest access
  $contents .= '<h3>' . t('Request Authorization') . '</h3>';
  if ($o_rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED) {
    $contents .= "This call needs EGA authentication.";
    if (!isset($a_var_setup['vars']['app_id'])) {
      $contents .= " WARNING! check vars";
    }
    unset($a_var_setup['vars']['app_id']);
  }

  $contents .= '<h3>' . t('Request Parameters') . '</h3>';

  unset($a_var_setup['vars']['app_id']);

  if ((isset($a_var_setup['vars']['is_app_admin'])) && ($a_var_setup['vars']['is_app_admin']['description'] == 'NOT USED.')) {
    unset($a_var_setup['vars']['is_app_admin']);
  }

  $header = array(
    array('data' => t('Parameter')),
    array('data' => t('Type')),
    array('data' => t('Description')),
    array('data' => t('Required')),
    array('data' => t('Default value')),
    array('data' => t('Max. length')),
  );

  // Sort on param.
  ksort($a_var_setup[mediamosa_rest_call::VARS]);
  $form = $rows = array();

  if (!isset($a_var_setup[mediamosa_rest_call::VARS])) {
    $a_var_setup[mediamosa_rest_call::VARS] = array();
  }
  // First required.
  foreach ($a_var_setup[mediamosa_rest_call::VARS] as $param => $a_item) {
    if (isset($a_item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $a_item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      $is_array = isset($a_item[mediamosa_rest_call::VAR_IS_ARRAY]) && $a_item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      $rows[] = array(
        $param . $is_array,
        $a_item[mediamosa_rest_call::VAR_TYPE],
        isset($a_item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $a_item[mediamosa_rest_call::VAR_DESCRIPTION] : '',
        t('Required'),
        isset($a_item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $a_item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        '-',
      );
    }
  }

  // Not required.
  foreach ($a_var_setup[mediamosa_rest_call::VARS] as $param => $a_item) {
    if (!isset($a_item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $a_item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      $is_array = isset($a_item[mediamosa_rest_call::VAR_IS_ARRAY]) && $a_item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      $rows[] = array(
        $param . $is_array,
        $a_item[mediamosa_rest_call::VAR_TYPE],
        isset($a_item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $a_item[mediamosa_rest_call::VAR_DESCRIPTION] : '',
        t('Optional'),
        isset($a_item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $a_item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        '-',
      );
    }
  }

  $contents .= theme('table', $header, $rows);

  $contents .= '<h3>' . t('Response fields') . '</h3>';

  $contents .= '<h3>' . t('Example Request') . '</h3>';
  $contents .= '<p>' . str_replace("\n\n", "</p>\n<br>\n<p>", $o_rest_call->example_request) . '</p>';

  $contents .= '<h3>' . t('Example response') . '</h3>';
  $contents .= '<p>' . str_replace("\n\n", "</p>\n<br>\n<code>", $o_rest_call->example_response) . '</code>';

  return $contents;
}
