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
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  // Skip internal calls.
  foreach ($rest_calls as $key => $rest_call) {
    foreach ($rest_call as $method => $value) {


      if (isset($value['access']) && ($value['access'] == 1)) {
        unset($rest_calls[$key][$method]);
      }
      if (isset($value['status']) && ($value['status'] == 'deprecated')) {
        unset($rest_calls[$key][$method]);
      }
    }
  }

  // Sort the rest calls on URI.
  ksort($rest_calls);

  $item_limit = 128;
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
      $has_example_request = !empty($rest_call[mediamosa_rest_call::EXAMPLE_REQUEST]);
      $has_example_response = !empty($rest_call[mediamosa_rest_call::EXAMPLE_RESPONSE]);

      $warnings = array();
      if (!$has_example_request) {
        $warnings[] = '<sup>1</sup>';
      }
      if (!$has_example_response) {
        $warnings[] = '<sup>2</sup>';
      }

      $rows[] = array(
        l('/' . $uri, 'admin/mediamosa/browse/restcall/' . str_replace('/', '-', $uri) . '/' . urlencode($method)) . ' ' . implode(' ', $warnings),
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
    '#markup' => theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('No REST calls found.'))),
  );

  $form['sup'] = array(
    '#markup' => implode(', ', array(t('<div align="right"><sup><small>1) missing example request</small>'), t('<small>2) missing example response</small></sup><div>'))),
  );

  $form['pager_bottom'] = array(
    '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
  );

  $form['total'] = array(
    '#markup' => 'total found: ' . $item_count_total,
  );

  return $form;
}

/**
 * View specific details of a REST call.
 */
function mediamosa_maintenance_browse_restcall_details($uri, $method) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administer'), 'admin');
  $breadcrumb[] = l(t('MediaMosa 2'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Rest calls browser'), 'admin/mediamosa/browse/restcall');
  drupal_set_breadcrumb($breadcrumb);

  $uri = str_replace('-', '/', $uri);
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');
  $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
  $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

  if (!isset($rest_calls[$uri][$method])) {
    return t('Rest call not found.');
  }

  // Add URI and Method to the array.
  $rest_calls[$uri][$method][mediamosa_rest_call::URI] = $uri;
  $rest_calls[$uri][$method][mediamosa_rest_call::METHOD] = $method;

  // Enrich the rest call with missing defaults.
  $a_rest_call = mediamosa_rest::set_default_rest_call($rest_calls[$uri][$method]);

  // Create class from it.
  $rest_call = new $rest_calls[$uri][$method][mediamosa_rest_call::CLASS_NAME]($a_rest_call);

  // Check if override for class selection is here.
  if (method_exists($rest_call, 'get_object_rest_call')) {
    $rest_call = $rest_call->get_object_rest_call($a_rest_call, isset($a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES]) ? $a_rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES] : array());
  }

  // Set as uri values, so we dont fail checks.
  if (isset($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES])) {
    foreach ($rest_calls[$uri][$method][mediamosa_rest_call::DEFAULT_PARAMS_VALUES] as $param => $value) {
      $rest_call->{mediamosa_rest_call::URI_PARAMS}[$param] = $value;
    }
  }

  // Turn off so we can call the get_var_setup without problems.
  // Although get_var_setup should not test variables.
  $rest_call->set_check_for_unspecified(FALSE);

  // Get the var setup for this call.
  $var_setup = $rest_call->get_var_setup();

  // Setup the title with the REST call URI.
  drupal_set_title(
    t('Viewing details rest call URI @uri, method [@method]',
      array(
      '@uri' => '/' . $uri,
      '@method' => $method
      )
    )
  );

  $warnings = array();

  if ($rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED) {
    if (!isset($var_setup['vars']['app_id'])) {
      $warnings[] = 'Check vars, missing app_id.';
    }
  }

  // We dont want to show the app_id to the outside.
  unset($var_setup['vars']['app_id']);

  if ((isset($var_setup['vars']['is_app_admin'])) && ($var_setup['vars']['is_app_admin']['description'] == 'NOT USED.')) {
    unset($var_setup['vars']['is_app_admin']);
  }

  $form = array();

  $form['restcall_doc_template'] = array(
    '#theme' => 'restcall_doc',
    '#rest_call' => $rest_call,
    '#var_setup' => $var_setup,
    '#warnings' => $warnings,
  );

  return $form;
}

/**
 * Process variables for restcall-doc.tpl.php.
 *
 * @see restcall-doc.tpl.php
 */
function template_preprocess_restcall_doc(&$variables) {
  $rest_call = $variables['rest_call'];
  $var_setup = $variables['var_setup'];

  $variables['rest_call']->request_authorization = t('This call can be called without any authentication.');

  if ($rest_call->access >= mediamosa_rest_call::ACCESS_AUTHENTICATED && empty($rest_call->{mediamosa_rest_call::BEHAVE_AS_EXTERNAL})) {
    $variables['rest_call']->request_authorization = t('This call needs EGA authentication.');
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
  ksort($var_setup[mediamosa_rest_call::VARS]);
  $form = $rows = array();

  if (!isset($var_setup[mediamosa_rest_call::VARS])) {
    $var_setup[mediamosa_rest_call::VARS] = array();
  }
  // First required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) && $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      $rows[] = array(
        $param . $is_array,
        mediamosa_type::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '',
        t('Required'),
        isset($item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        '-',
      );
    }
  }

  // Not required.
  foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $item) {
    if (!isset($item[mediamosa_rest_call::VAR_IS_REQUIRED]) || $item[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      $is_array = isset($item[mediamosa_rest_call::VAR_IS_ARRAY]) && $item[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES ? '[]' : '';

      $rows[] = array(
        $param . $is_array,
        mediamosa_type::typeAlias2Type($item[mediamosa_rest_call::VAR_TYPE]),
        isset($item[mediamosa_rest_call::VAR_DESCRIPTION]) ? $item[mediamosa_rest_call::VAR_DESCRIPTION] : '',
        t('Optional'),
        isset($item[mediamosa_rest_call::VAR_DEFAULT_VALUE]) ? $item[mediamosa_rest_call::VAR_DEFAULT_VALUE] : '',
        '-',
      );
    }
  }

  // Parameter list.
  $variables['parameters'] = theme(
    'table',
    array(
      'header' => $header,
      'rows' => $rows,
      'empty' => t('This REST call has no input parameters.'),
    )
  );

  // Response fields.
  $header = array(
    array('data' => t('Field')),
    array('data' => t('Description')),
  );

  $rows = array();
  if (!empty($rest_call->response_fields)) {
    foreach ($rest_call->response_fields as $key => $value) {
      $rows[] = array($key, $value);
    }
  }

  // Hide table when response fields is empty.
  if (!empty($rows)) {
    $variables['response_fields'] = theme(
      'table',
      array(
        'header' => $header,
        'rows' => $rows,
        'empty' => t('No response fields found.')
      )
    );
  }

  if (empty($variables['rest_call']->example_request)) {
    $variables['warnings'][] = t('No example request specified.');
  }

  if (empty($variables['rest_call']->example_response)) {
    $variables['warnings'][] = t('No example response specified.');
  }
}
