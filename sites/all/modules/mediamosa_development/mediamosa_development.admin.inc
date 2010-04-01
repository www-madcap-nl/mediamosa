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
 * Admin related forms and functions.
 */

/**
 * Insert the output of the REST call into a text area.
 */
function _ajax_execute_rest_call() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  // Get the select REST call.
  $uri_method = isset($_SESSION['mediamosa_development_params']['uri']) ? $_SESSION['mediamosa_development_params']['uri'] : '';

  if (empty($uri_method)) {
    return array('#type' => 'ajax', '#commands' => array(), '#header' => FALSE);
  }

  list($uri, $method) = explode('|', $uri_method);

  $var_setup = mediamosa_rest::get_var_setup($_SESSION['mediamosa_development_params']['rest_call']);
  $data = $uri_rebuild = $uri_vars = $errors = array();

  // fill in the uri with the values (when needed).
  foreach (explode('/', $uri) as $uri_var) {
    $uri_var = trim($uri_var, '# ');
    if ($uri_var{0} == '$') {
      if (!isset($form_state['input']['_' . substr($uri_var, 1)]) || $form_state['input']['_' . substr($uri_var, 1)] == '') {
        $errors[] = t('Missing value for @name', array('@name' => substr($uri_var, 1)));
        continue;
      }

      // So we know the urivars.
      $uri_vars[] = substr($uri_var, 1);

      // To rebuild.
      $uri_rebuild[] = $form_state['input']['_' . substr($uri_var, 1)];
    }
    else {
      $uri_rebuild[] = $uri_var;
    }
  }

  // Rebuild uri.
  $uri = implode('/', $uri_rebuild);

  // Get the vars.
  foreach ($var_setup['vars'] as $name => $var) {
    if (!isset($form_state['input']['_' . $name]) || $form_state['input']['_' . $name] == '') {
      continue;
    }

    if (in_array($name, $uri_vars)) {
      continue;
    }

    // Store the value.
    $data[$name] = $form_state['input']['_' . $name];
  }

  // Get the selected app.
  $app_id = $form_state['input']['client_application'];
  $username = NULL;
  $password = NULL;

  if ($app_id) {
    $app = mediamosa_app::get_by_appid($app_id);

    $username = $app[mediamosa_app_db::APP_NAME];
    $password = $app[mediamosa_app_db::SHARED_KEY];
  }

  $commands = array();

  // Run same on mediamosa 1.7?
  $run_on_mm_17 = $form_state['input']['run_on_mm_17'] == 1;
  // Build and render the new output, then return it in JSON format.
  $form_state = array('programmed' => FALSE, 'values' => array());
  $form = form_builder($form['form_id']['#value'], $form, $form_state);

  if (empty($errors)) {
    // Create mediamosa_connector.
    $mediamosa_connector = new mediamosa_connector($username, $password);

    // Get response.
    $response = $mediamosa_connector->request($uri, array('method' => $method, 'data' => $data));

    if (empty($response) || empty($response->data)) {
      $form['rest_result']['output']['#value'] = t('Check mediamosa connector settings, no output received.');
    }
    else {
      $form['rest_result']['output']['#value'] = $response->data;
    }

    if ($run_on_mm_17) {
      $mediamosa_development_mediamosa_17_rest_url = variable_get('mediamosa_development_mediamosa_17_rest_url', '');

      if (!empty($mediamosa_development_mediamosa_17_rest_url)) {
        // Create mediamosa_connector.
        $mediamosa_connector = new mediamosa_connector($username, $password, $mediamosa_development_mediamosa_17_rest_url);

        // Get response.
        $response = $mediamosa_connector->request($uri, array('method' => $method, 'data' => $data));

        if (empty($response) || empty($response->data)) {
          $response = t('Check MediaMosa 1.7 settings, unable to retrieve data');
        }
        else {
          $response = $response->data;
        }

        $form['rest_result']['output2']['#type'] = 'textarea';
        $form['rest_result']['output2']['#value'] = $response;
      }
    }

  }
  else {
    $form['rest_result']['output']['#value'] = implode("\n", $errors);
  }

  $commands[] = ajax_command_replace('', drupal_render($form['rest_result']));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * The page to execute a REST call.
 */
function mediamosa_development_rest_page_form() {
  $build = array();

  $build['restcall'] = drupal_get_form('mediamosa_development_rest_form');

  return $build;
}

/**
 * Set a REST uri.
 *
 * @param string $uri
 * @param string $method
 */
function _mediamosa_development_set_rest_call($uri, $method) {
  // Get session.
  $_SESSION['mediamosa_development_params']['uri'] = str_replace('-', '/', $uri) . '|' . $method;

  drupal_goto('admin/mediamosa/config/development/rest');
}

/**
 * The form to execute a REST call.
 */
function mediamosa_development_rest_form() {

  // Get session.
  $session = isset($_SESSION['mediamosa_development_params']) ? $_SESSION['mediamosa_development_params'] : array();

  drupal_add_css(drupal_get_path('module', 'mediamosa_development') . '/mediamosa_development.css');

  // Get all the REST calls.
  $rest_calls = module_invoke_all('mediamosa_register_rest_call');

  $rest_call_options = array('' => 'Select Rest call');
  foreach ($rest_calls as $uri => $rest_call) {
    foreach ($rest_call as $method => $rest_call_data) {
      $rest_call_options[$uri . '|' . $method] = $uri . ' [' . $method . ']';
    }
  }

  ksort($rest_call_options);

  $form = array();

  $form['rest'] = array(
    '#type' => 'fieldset',
    '#title' => t('REST call'),
    '#weight' => -10,
  );

  $apps = array();
  if (module_exists('mediamosa_connector')) {
    $apps[0] = t('Use MediaMosa Connector to connect');
  }

  $apps_2 = mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_ID, 'ASC')->fetchAllKeyed();
  foreach ($apps_2 as $app_id => $app_name) {
    $apps[$app_id] = strtr('@name (App ID @app_id)', array('@name' => $app_name, '@app_id' => $app_id));
  }

  if (empty($apps)) {
    $form['rest']['client_application'] = array(
      '#markup' => t('No client applications found or MediaMosa connector setup.'),
    );
  }

  $form['rest']['client_application'] = array(
    '#type' => 'select',
    '#title' => 'REST call URI',
    '#options' => $apps,
    '#default_value' => '',
  );

  $mediamosa_development_mediamosa_17_rest_url = variable_get('mediamosa_development_mediamosa_17_rest_url', '');
  if (!empty($mediamosa_development_mediamosa_17_rest_url)) {
    $form['rest']['run_on_mm_17'] = array(
      '#type' => 'checkbox',
      '#title' => t('Run REST call on MediaMosa 1.7.2 to compare output.'),
      '#default_value' => 0,
    );
  }

  if (empty($session['uri'])) {
    $form['rest']['uri'] = array(
      '#type' => 'select',
      '#title' => 'REST call URI',
      '#options' => $rest_call_options,
      '#default_value' => '',
    );

    $form['rest']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Select'),
    );
  }
  else {
    list($uri, $method) = explode('|', $session['uri']);

    // Get all the REST calls docs.
    $rest_call_docs = module_invoke_all('mediamosa_register_rest_call_doc');

    // Get the doc.
    $rest_call_doc = isset($rest_call_docs[$uri][$method]) ? $rest_call_docs[$uri][$method] : array();

    $descriptions = array(
      !empty($rest_call_doc[mediamosa_rest_call::TITLE]) ? '<b>' . $rest_call_doc[mediamosa_rest_call::TITLE] . '</b>' : t('No title found'),
      !empty($rest_call_doc[mediamosa_rest_call::DESCRIPTION]) ? $rest_call_doc[mediamosa_rest_call::DESCRIPTION] : t('No description found'),
      '',
      l(t('@uri [@method]', array('@uri' => $uri, '@method' => $method)), strtr('admin/mediamosa/browse/restcall/@uri/@method', array('@uri' => str_replace('/', '-', $uri), '@method' => $method))),
    );

    $form['rest']['description'] = array(
      '#markup' => '<p>' . implode('<br />', $descriptions) . '</p>',
    );

    $form['rest']['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Clear URI selection'),
    );

    $form['rest']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Execute @uri [@method]', array('@uri' => $uri, '@method' => $method)),
      '#ajax' => array(
        'event' => 'click',
        'wrapper' => 'rest_call_output',
        'path' => 'admin/mediamosa/config/development/js/restcall',
      ),
      '#weight' => 10,
    );

    $form['rest_result'] = array(
      '#type' => 'fieldset',
      '#title' => t('REST Call output'),
      '#weight' => 10,
      '#prefix' => '<div id="rest_call_output">',
      '#suffix' => '</div>',
    );

    $form['rest_result']['output'] = array(
      '#type' => 'textarea',
      '#title' => t('Output'),
      '#default_value' => '',
    );

    $form['rest_result']['output_mm17'] = array(
      '#type' => 'hidden',
      '#title' => t('Output 2'),
      '#default_value' => '',
      '#attributes' => array('class' => 'output_mm17'),
    );

    $form = _mediamosa_build_form($form);
  }

  return $form;
}

/**
 * Submit of the rest form.
 */
function mediamosa_development_rest_form_submit($form, &$form_state) {

  switch ($form_state['values']['op']) {
    case t('Clear URI selection'):
      $_SESSION['mediamosa_development_params']['uri'] = '';
      break;

    case t('Select'):
      // Set uri in session.
      $_SESSION['mediamosa_development_params']['uri'] = isset($form_state['input']['uri']) ? $form_state['input']['uri'] : '';

      if (empty($_SESSION['mediamosa_development_params']['uri'])) {
        break;
      }

      list($uri, $method) = explode('|', $_SESSION['mediamosa_development_params']['uri']);

      // Find rest_call.
      $_SESSION['mediamosa_development_params']['rest_call'] = mediamosa_rest::get_uri_definition($uri, $method);
      assert(!empty($_SESSION['mediamosa_development_params']['rest_call']));

      $form = _mediamosa_build_form($form);
      break;

    case t('Execute'):
      break;
  }
}

function _mediamosa_build_form($form) {
  if (empty($_SESSION['mediamosa_development_params']['rest_call'])) {
    return $form;
  }

  $var_setup = mediamosa_rest::get_var_setup($_SESSION['mediamosa_development_params']['rest_call']);

  // Clear them.
  $required = $optional = array();

  $skip = array('app_id');

  // Sort vars on name.
  ksort($var_setup['vars']);

  // First the required.
  $weight = 0;
  foreach ($var_setup['vars'] as $name => $var) {
    if (!isset($var[mediamosa_rest_call::VAR_IS_REQUIRED]) || $var[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_NO) {
      continue;
    }

    // Some need to be skipped.
    if (in_array($name, $skip)) {
      continue;
    }

    // Skip it when its not used. @todo remove
    if ($name == mediamosa_rest_call::IS_APP_ADMIN && $var[mediamosa_rest_call::VAR_DESCRIPTION] == 'NOT USED.') {
      continue;
    }

    $item = _build_param($name, $var, true);

    if (!empty($item)) {
      $item['#weight'] = $weight++;
      $required['_' .$name] = $item; // prefix with '_' so we dont have problems with double names.
    }
  }

  // Second the optional.
  $weight = 0;
  foreach ($var_setup['vars'] as $name => $var) {
    if (isset($var[mediamosa_rest_call::VAR_IS_REQUIRED]) && $var[mediamosa_rest_call::VAR_IS_REQUIRED] == mediamosa_rest_call::VAR_IS_REQUIRED_YES) {
      continue;
    }

    // Some need to be skipped.
    if (in_array($name, $skip)) {
      continue;
    }

    // Skip it when its not used. @todo remove
    if ($name == mediamosa_rest_call::IS_APP_ADMIN && $var[mediamosa_rest_call::VAR_DESCRIPTION] == 'NOT USED.') {
      continue;
    }

    $item = _build_param($name, $var, false);

    if (!empty($item)) {
      $item['#weight'] = $weight++;
      $optional['_' . $name] = $item;
    }
  }

  if (empty($required) && empty($optional)) {
    $form['rest']['required'] = array('#markup' => '<p>' . t('No parameters for this REST call.') . '</p>');
  }
  else {
    if (!empty($required)) {
      $form['rest']['required'] = array(
        '#type' => 'fieldset',
        '#title' => t('Required parameters'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );

      $form['rest']['required'] = isset($form['rest']['required']) ? array_merge($required, $form['rest']['required']) : $required;
    }
    else {
      $form['rest']['required'] = array('#markup' => '<p>' . t('No required parameters.') . '</p>');
    }

    if (!empty($optional)) {
      $form['rest']['optional'] = array(
        '#type' => 'fieldset',
        '#title' => t('Optional parameters'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $form['rest']['optional'] = array_merge($optional, $form['rest']['optional']);
    }
  }

  return $form;
}

/**
 * Helper for creating param form item.
 *
 * @param string $name
 * @param array $var
 */
function _build_param($name, $var, $required) {
  $item = array();

  $type = mediamosa_type::typeAlias2Type($var[mediamosa_rest_call::VAR_TYPE]);
  $is_array = isset($var[mediamosa_rest_call::VAR_IS_ARRAY]) && $var[mediamosa_rest_call::VAR_IS_ARRAY] == mediamosa_rest_call::VAR_IS_ARRAY_YES;

  // Title.
  $title = $name . ($is_array ? '[]' : '');
  $options = array();

  if (isset($var[mediamosa_rest_call::VAR_ALLOWED_VALUES]) && !empty($var[mediamosa_rest_call::VAR_ALLOWED_VALUES])) {
    if (!$required) {
      $options[''] = t('Make selection');
      $var[mediamosa_rest_call::VAR_DEFAULT_VALUE] = t('Make selection');
    }

    foreach ($var[mediamosa_rest_call::VAR_ALLOWED_VALUES] as $value) {
      $options[$value] = $value;
    }

    // Go pass the type.
    $type = 'hack_selection';
  }

  // Description.
  $description = $var[mediamosa_rest_call::VAR_DESCRIPTION];

  switch ($type) {
    case 'hack_selection':
      $item = array(
        '#weight' => '10',
        '#type' => 'select',
        '#title' => $title,
        '#description' => $description,
        '#options' => $options,
      );
      break;

    case mediamosa_type::TYPE_BOOL:
      $item = array(
        '#weight' => '10',
        '#type' => 'select',
        '#title' => $title,
        '#description' => $description,
        '#options' => array('false' => t('false'), 'true' => t('true')),
      );

      if (isset($var[mediamosa_rest_call::VAR_DEFAULT_VALUE])) {
        $var[mediamosa_rest_call::VAR_DEFAULT_VALUE] = drupal_strtolower($var[mediamosa_rest_call::VAR_DEFAULT_VALUE]);
      }
      else {
        $var[mediamosa_rest_call::VAR_DEFAULT_VALUE] = 'false';
      }

      break;
    case mediamosa_type::TYPE_APP_ID:
    case mediamosa_type::TYPE_INT:
    case mediamosa_type::TYPE_UINT:

    case mediamosa_type::TYPE_STRING:
    case mediamosa_type::TYPE_ALPHA:
    case mediamosa_type::TYPE_ALPHA_NUM:
      $item = array(
        '#weight' => '10',
        '#type' => 'textfield',
        '#title' => $title,
        '#description' => $description,
      );

      break;
    default:
      drupal_set_message(t('No form creation found for @name with type @type.', array('@name' => $name, '@type' => $type)), 'error');
      break;
  }

  if (!empty($item)) {
    if (!empty($var[mediamosa_rest_call::VAR_DEFAULT_VALUE])) {
      $item['#default_value'] = $var[mediamosa_rest_call::VAR_DEFAULT_VALUE];
    }
  }
  return $item;
}
