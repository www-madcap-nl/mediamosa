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
 * Collection browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_COLLECTION', 'mediafile_browse_collection');

/**
 * Build the form for browsing collections.
 */
function mediamosa_browse_collection_page($op = 'list') {

  $build = array();

  $build['collection_filter'] = drupal_get_form('mediamosa_browse_collection_filter_form');
  $build['collection_list'] = drupal_get_form('mediamosa_browse_collection_list_form');

  return $build;
}

/**
 * Form builder; Return form for collection filters.
 *
 * @see mediamosa_browse_collection_filter_form_submit()
 */
function mediamosa_browse_collection_filter_form() {

  // Get session.
  $session = _mediamosa_browse_collection_session();

  $form = array();

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="searchparameters">',
    '#suffix' => '</div>',
  );

  $form['filters']['operator'] = array(
    '#type' => 'select',
    '#title' => t('Search criteria operator'),
    '#description' => t("Filter on all search criteria ('AND') or a least one criterium ('OR')."),
    '#options' => array('and' => t('AND'), 'or' => t('OR')),
    '#default_value' => isset($session['operator']) ? $session['operator'] : 'and',
  );

  $form['filters']['searchparameters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search criteria'),
  );

  $form['filters']['searchparameters']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add search item'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'searchparameters',
      'path' => 'admin/mediamosa/browse/js/collection/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 10,
  );

  $form['filters']['searchparameters']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'searchparameters',
      'path' => 'admin/mediamosa/browse/js/collection/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 11,
  );


  $form['filter']['apply'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  $form = _mediamosa_browse_collection_searchparameters($form);

  return $form;
}

/**
 * Insert the search form into a text area.
 */
function _ajax_browse_collection_search_parameters() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  // Fix drupal notice.
  $form_state['values'] = isset($form_state['values']) ? $form_state['values'] : array();
  $form = form_builder('', _mediamosa_browse_collection_searchparameters($form, $form_state), $form_state);

  $commands[] = ajax_command_replace('', drupal_render($form['filters']));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * Enrich the form with the filter items.
 *
 * @param array $form
 * @param array $form_state
 */
function _mediamosa_browse_collection_searchparameters($form, $form_state = array()) {

  // Get session.
  $session = _mediamosa_browse_collection_session($form_state);

  if (!isset($form_state['input']['filters'])) {
    $form_state['input']['filters'] = isset($session['filters']) ? $session['filters'] : array();
  }

  $op = isset($form_state['input']['op']) ? $form_state['input']['op'] : '';

  switch ($op) {
    case t('Reset'):
      $session['operator'] = 'and';
      $session['filters'] = array();
      $form_state['input']['filters'] = array();

    case t('Add search item'):
      $form_state['input']['filters'][] = array('item' => '', 'item_value' => '');
      break;

    default:
      if (empty($form_state['input']['filters'])) {
        $form_state['input']['filters'][] = array('item' => '', 'item_value' => '');
      }
      break;

    case t('Apply filter'):
      break;
  }

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;

  unset($form['filters']['searchparameters']['filters']);
  foreach ($form_state['input']['filters'] as $x => $filter) {
    $form_filter = _mediamosa_browse_searchparameters_helper($x, $filter);

    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item]'] = $form_filter['item'];
    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item_value]'] = $form_filter['item_value'];
  }

  return $form;
}

/**
 * Process result from collection filter form.
 */
function mediamosa_browse_collection_filter_form_submit($form, $form_state) {
  _mediamosa_browse_collection_session($form_state);
}

/**
 * Form builder; collection listing page.
 *
 */
function mediamosa_browse_collection_list_form() {
  // Get session.
  $session = _mediamosa_browse_collection_session();

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenance_browse.css');

  $limit = $session['limit'];
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $item_count_total = 0;

  $header = array(
    array('field' => 'title', 'data' => t('Title'), 'sort' => 'asc'),
    array('field' => 'app_id', 'data' => t('App ID'), 'class' => array('mmappid')),
    array('field' => 'changed', 'data' => t('Last modified'), 'class' => array('mmdate')),
  );

  $result = _mediamosa_browse_collection_execute();
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  foreach ($result['items'] as $row) {
    $title = empty($row['title']) ? NULL : check_plain($row['title']);
    $rows[] = array(
      theme('l_mediamosa_collection', array('title' => $title, 'id' => $row[mediamosa_collection_db::ID])),
      array('data' => check_plain($row['app_id']), 'class' => array('mmappid')),
      array('data' => theme('mediamosa_maintenance_date', array('datetime' => $row[mediamosa_collection_db::CHANGED])), 'class' => array('mmdate')),
    );
  }

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No collections found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Top
    '#title' => t('Collections (@total)', array('@total' => $item_count_total)),
    '#description' => t('Collections matching the search criteria above are listed below. Click on a collection title below to view details.'),
    // Total note.
    '#caption_bottom' => t('Collections found: @total', array('@total' => $item_count_total))
  );

  return $form;
}

/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_collection_execute() {
  // Get session.
  $session = _mediamosa_browse_collection_session();

  // What to sort on.
  $sort = $session['sort'];
  $order = $session['order'];
  $limit = $session['limit'];
  $page = $session['page'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Last modified'):
      $order = 'changed';
      break;

    case t('Title'):
    default:
      $order = 'title';
      break;
  }

  $params = array();

  $cqls = array();

  $cql_session = _mediamosa_browse_cql_build($session);
  if (!empty($cql_session)) {
    $cqls[] = $cql_session;
  }

  // Add sort.
  $cqls[] = strtr('sortBy @order/@sort', array('@order' => $order, '@sort' => drupal_strtolower($sort) == 'asc' ? 'ascending' : 'descending'));

  // Add params.
  $params['cql'] = implode(' ', $cqls);
  $params['limit'] = $limit;
  $params['offset'] = $page * $limit;

  // Do the rest call.
  return mediamosa_response_connector::static_do_restcall_drupal('collection', 'GET', $params);
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_collection_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] : array();

  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Title'),
    'sort' => 'asc',
    'page' => 0,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
  );

  // Process filter.
  if (!empty($filter_form_state['input']['operator'])) {
    $session['operator'] = $filter_form_state['input']['operator'];
  }

  if (!empty($filter_form_state['input']['filters'])) {
    $session['filters'] = $filter_form_state['input']['filters'];
  }

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;

  return $session;
}

/**
 * View the details of an collection.
 */
function mediamosa_maintenance_browse_collection_details($coll_id) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administer'), 'admin');
  $breadcrumb[] = l(t('MediaMosa 2'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Collection browser'), 'admin/mediamosa/browse/collection');
  drupal_set_breadcrumb($breadcrumb);

  // Get the collection.
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('collection/@coll_id', array('@coll_id' => rawurlencode($coll_id))) , 'GET');

  if ($response['header']['request_result_id'] != 601) {
    return '<h1>Collection not found</h1>';
  }

  $collection = reset($response['items']);

  $variables = array(
    'description' => t('Details for this collection are shown below.'),
    'properties' => array(
      t('Title') => $collection[mediamosa_collection_db::TITLE],
      t('Description') => $collection[mediamosa_collection_db::DESCRIPTION],
      t('App ID') => $collection[mediamosa_collection_db::APP_ID],
      t('Owner ID') => $collection[mediamosa_collection_db::OWNER_ID],
      t('Group ID') => $collection[mediamosa_collection_db::GROUP_ID],
    ),
  );

  $page = isset($_GET['page']) ? $_GET['page'] : 0;

  // Get the possible assets in this collection.
  $params = array('limit' => 10, 'offset' => $page * 10);
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('collection/@coll_id/asset', array('@coll_id' => rawurlencode($coll_id))) , 'GET', $params);

  if ($response['header']['request_result_id'] == 601) {
    $variables['collection_assets']['title'] = t('Assets (@total)', array('@total' => $response['header']['item_count_total']));
    $variables['collection_assets']['coll_id'] = $coll_id;

    if ($response['header']['item_count_total'] > 0) {
      $variables['collection_assets']['description'] = t('Collection <em>@name</em> contains the following assets.', array('@name' => $collection[mediamosa_collection_db::TITLE]));

      $variables['collection_assets']['item_count_total'] = $response['header']['item_count_total'];
      $variables['collection_assets']['item_offset'] = $response['header']['item_offset'];
      $variables['collection_assets']['page'] = $page;
      $variables['collection_assets']['assets'] = array();

      foreach ($response['items'] as $asset) {
        $variables['collection_assets']['assets'][] = $asset;
      }
    }
    else {
      $variables['collection_assets']['description'] = t('Collection <em>@name</em> does not contain any assets.', array('@name' => $collection[mediamosa_collection_db::TITLE]));
    }
  }

  $build['properties'] = theme('mediamosa_maintenance_browse_properties', $variables);

  return implode('', $build);
}



/**
 * Helper function for the collection filter form

function _mediamosa_maintenance_browse_collection_list_filter() {
  $search_terms = array(
    0 => '',
    'title' => t('Title'),
    'description' => t('Description'),
    'owner_id' => t('Owner ID'),
    'group_id' => t('Group ID'),
  );

  // Sort the array
  asort($search_terms);

  return $search_terms;
}

function _mediamosa_maintenance_browse_collection_get_cql_from_session() {
  $filters = $data = array();

  // Iterate all items stored in the form submit function
  foreach ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION] as $key => $filter) {
    switch ($key) {
      case 'app_id':
        $filters['app_id'] = explode(' ', $filter);
        break;

      case 'coll_id':
        $filters['coll_id'][] = $filter;
        break;

      case 'filters':
        foreach ($filter as $filter_item) {
          $filter_key = reset(array_keys($filter_item));
          $filter_value = reset(array_values($filter_item));

          // Avoid duplicates
          if ((isset($filters[$filter_key]) && !in_array($filter_value, $filters[$filter_key])) || !isset($filters[$filter_key])) {
            $filters[$filter_key][] = addslashes($filter_value);
          }
        }
        break;

    }
  }

  // App id's must be passed through separate GET parameters
  $data['app_ids'] = $filters['app_id'];
  unset($filters['app_id']);

  // Construct the cql
  $cql = array();
  foreach ($filters as $filter_group => $filter) {
    $cql[] = sprintf('%s == "%s"', $filter_group, implode(' ', $filter));
  }
  $data['cql'] = '('. implode(" AND ", $cql) .')';

  // Add the optional sort order
  if ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['tablesort']) {
    $sortby = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['tablesort']['sql'];
    $sortorder = ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['tablesort']['sort'] == 'desc') ? 'descending' : 'ascending';
    $data['cql'] .= 'sortBy '. $sortby .'/'. $sortorder;
  }

  return $data;
}


/**
 * The browse form which also contains a list of browsable collections.

function mediamosa_maintenance_browse_collection_form() {
  // Gather and prepare some variables.
  $form = array();
  $item_limit = 50;
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  // Gather all apps
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME, 'ASC') as $app) {
    $all_apps_raw[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::APP_NAME];
  }
  $all_apps = array(implode(' ', array_keys($all_apps_raw)) => t('<All applications>'));
  $all_apps += $all_apps_raw;

  $app_id = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['app_id'])) ? $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['app_id'] : reset(array_keys($all_apps));
  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['app_id'] = $app_id;
  $form['app_id'] = array(
    '#type' => 'select',
    '#options' => $all_apps,
    '#title' => t('Application'),
    '#default_value' => $app_id,
  );

  // Get the current filter count
  $filter_count = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['filters'])) ? count($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['filters']) : 0;

  // Add one empty filter item
  $filter_count++;

  // Get the maximum number of filters
  $filter_max = variable_get('mediamosa_maintenance_browse_filter_max', 25);

  // Load and set all javascript related items
  drupal_add_js(drupal_get_path('module', 'mediamosa_maintenance_browse') .'/mediamosa_maintenance_browse.js');
  drupal_add_js(
    array(
      'mediamosa_maintenance_browse' => array(
        'filter_count' => $filter_count,
        'filter_max' => $filter_max,
      ),
    ),
    'setting'
  );

  $filter_types = _mediamosa_maintenance_browse_collection_list_filter();
  for ($i = 1; $i <= $filter_max; $i++) {
    $display = ($i <= $filter_count) ? '' : 'none';
    $extra_class = '';
    if ($i == 1) {
      $extra_class .= ' first';
    }
    elseif ($i == $filter_max) {
      $extra_class .= ' last';
    }

    $form['filter_type_'. $i] = array(
      '#prefix' => '<div id="mediamosa-browse-collection-filter-'. $i .'" style="display: '. $display .'" class="mediamosa-browse-collection-filter'. $extra_class .'">',
      '#type' => 'select',
      '#options' => $filter_types,
      '#title' => t('Filter item @count', array('@count' => $i)),
    );

    $form['filter_value_'. $i] = array(
      '#type' => 'textfield',
      '#title' => '',
      '#suffix' => '</div>',
    );

    // Restore submitted values
    if (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['filters'][($i - 1)])) {
      $info = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['filters'][($i - 1)];
      $form['filter_type_'. $i]['#default_value'] = reset(array_keys($info));
      $form['filter_value_'. $i]['#default_value'] = reset(array_values($info));
    }
  }

  // What's a form without a submit button?
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  $form['add_filter_row'] = array(
    '#type' => 'submit',
    '#value' => t('Add search item'),
    '#suffix' => '<a href="#" id="edit-clear">Clear form</a>',
  );

  // Only show results on GET requests to prevent Drupal from fetching the results on form validation
  if ($_SERVER['REQUEST_METHOD'] != 'POST') {

    $header = array(
      array('field' => 'title', 'data' => t('Title')),
      array('field' => 'app_id', 'data' => t('App ID')),
      array('field' => 'changed', 'data' => t('Last modified')),
    );
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['tablesort'] = (isset($_GET['order'])) ? tablesort_init($header) : FALSE;

    // Fetch the collections
    $cql_data = _mediamosa_maintenance_browse_collection_get_cql_from_session();
    $data = array(
      'limit' => $item_limit,
      'offset' => $offset,
      'cql' => $cql_data['cql'],
      'app_id' => $cql_data['app_ids'],
    );
    $response = _mediamosa_restcall('collection', 'GET', $data);

    // Extract the item count
    $item_count = (int)$response->header->item_count;
    $item_count_total = (int)$response->header->item_count_total;

    // Return the form if no collections were found
    if ($item_count_total == 0) {
      $form['results'] = array(
        '#markup' => '<p>'. t('No collections found...') .'</p>',
      );
      return $form;
    }

    // Correct the item limit if the count is less than it.
    if ($item_count_total < $item_limit) {
      $item_limit = $item_count_total;
    }
    drupal_set_title(t('Showing collections @from-@to of @total', array(
      '@from' => $offset + 1,
      '@to' => $offset + $item_count,
      '@total' => $item_count_total,
    )));

    $rows = array();
    foreach ($response->xpath('//items/item') as $collection) {
      $title = (isset($collection->title) && trim((string)$collection->title) != '') ? $collection->title : t('<title not set>');
      $rows[] = array(
        l($title, 'admin/mediamosa/browse/asset', array('query' => array('coll_id' => $collection->coll_id))),
        $collection->app_id,
        $collection->changed,
      );
    }

    $form['pager_top'] = array(
      '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
    );

    $form['results'] = array(
      '#markup' => theme('table', array('header' => $header, 'rows' => $rows)),
    );

    $form['pager_bottom'] = array(
      '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
    );
  }

  return $form;
}

/**
 * The browse filter form submit function.

function mediamosa_maintenance_browse_collection_form_submit($form, $form_state) {
  $filters = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'filter_type_') === 0 && !is_numeric($value)) {
      $id = str_replace('filter_type_', '', $key);
      if (trim($form_state['values']['filter_value_'. $id]) != '') {
        $filters[] = array($value => trim($form_state['values']['filter_value_'. $id]));
      }
    }
  }

  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION] = array(
    'filters' => $filters,
    'app_id' => $form_state['values']['app_id'],
  );
}
*/