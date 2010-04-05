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
 * Asset browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_ASSET', 'mediafile_browse_asset');

/**
 * Build the form for browsing assets.
 */
function mediamosa_browse_asset_page($op = 'list') {

  // Get/set session.
  $session = _mediamosa_browse_asset_session();

  $build = array();

  $build['asset_filter'] = drupal_get_form('mediamosa_browse_asset_filter_form');
  $build['asset_list'] = drupal_get_form('mediamosa_browse_asset_list_form');

  return $build;
}

/**
 * Form builder; Return form for asset filters.
 *
 * @see mediamosa_browse_asset_filter_form_submit()
 */
function mediamosa_browse_asset_filter_form() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // Get all collections.
  $collections = array_merge(array(0 => ''), mediamosa_collection::getAll(array(mediamosa_collection_db::ID, mediamosa_collection_db::TITLE))->fetchAllKeyed());
  foreach ($collections as $coll_id => $name) {
    if (!$coll_id || trim($name) != '') {
      continue;
    }

    // Set empty titles.
    $collections[$coll_id] = t('<collection has no name> ID: @id', array('@id' => $coll_id));
  }

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

  $form['filters']['coll_id'] = array(
    '#type' => 'select',
    '#title' => t('Search within collection'),
    '#description' => t("Select collection to be searched. Leave empty for all collections."),
    '#options' => $collections,
    '#default_value' => isset($session['coll_id']) ? $session['coll_id'] : 0,
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
      'path' => 'admin/mediamosa/browse/js/asset/searchparameters',
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
      'path' => 'admin/mediamosa/browse/js/asset/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 11,
  );

  $form['filters']['apply'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  // CQL filter.
  $form['cqlfilter'] = array(
    '#type' => 'fieldset',
    '#title' => t('CQL Filter'),
    '#collapsible' => TRUE,
  );

  $form['cqlfilter']['cql'] = array(
    '#type' => 'textfield',
    //'#title' => t('CQL:'),
    '#description' => t('Enter a CQL search command here. Usage can be found in the Webservice document. Examples: "aut_app==5" or "is_protected == FALSE"'),
    '#default_value' => isset($session['cql']) ? $session['cql'] : '',
  );

  $form['cqlfilter']['apply_cql'] = array(
    '#type' => 'submit',
    '#value' => t('Apply CQL filter'),
  );

  $form = _mediamosa_browse_asset_searchparameters($form);

  return $form;
}

/**
 * Insert the search form into a text area.
 */
function _ajax_browse_asset_search_parameters() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  // Fix drupal notice.
  $form_state['values'] = isset($form_state['values']) ? $form_state['values'] : array();
  $form = form_builder('', _mediamosa_browse_asset_searchparameters($form, $form_state), $form_state);

  $commands[] = ajax_command_replace('', drupal_render($form['filters']));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * Enrich the form with the filter items.
 *
 * @param array $form
 * @param array $form_state
 */
function _mediamosa_browse_asset_searchparameters($form, $form_state = array()) {

  // Get session.
  $session = _mediamosa_browse_asset_session($form_state);

  if (!isset($form_state['input']['filters'])) {
    $form_state['input']['filters'] = isset($session['filters']) ? $session['filters'] : array();
  }

  $op = isset($form_state['input']['op']) ? $form_state['input']['op'] : '';

  switch ($op) {
    case t('Reset'):
      $session['operator'] = 'and';
      $session['coll_id'] = 0;
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
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] = $session;

  unset($form['filters']['searchparameters']['filters']);
  foreach ($form_state['input']['filters'] as $x => $filter) {
    $form_filter = _mediamosa_browse_searchparameters_helper($x, $filter);

    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item]'] = $form_filter['item'];
    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item_value]'] = $form_filter['item_value'];
  }

  return $form;
}

/**
 * Process result from asset filter form.
 */
function mediamosa_browse_asset_filter_form_submit($form, $form_state) {
  _mediamosa_browse_asset_session($form_state);
}

/**
 * Form builder; asset listing page.
 *
 */
function mediamosa_browse_asset_list_form() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

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

  $result = _mediamosa_browse_asset_execute();
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  foreach ($result['items'] as $row) {
    $rows[] = array(
      theme('l_mediamosa_asset', array('title' => _mediamosa_browse_asset_get_title($row), 'id' => $row[mediamosa_asset_db::ID])),
      array('data' => check_plain($row['app_id']), 'class' => array('mmappid')),
      array('data' => theme('mediamosa_maintenance_date', array('datetime' => $row[mediamosa_asset_db::VIDEOTIMESTAMP])), 'class' => array('mmdate')),
    );
  }

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No assets found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Top
    '#title' => t('Assets (@total)', array('@total' => $item_count_total)),
    '#description' => t('Assets matching the search criteria above are listed below. Click on a asset title below to view details.'),
    // Total note.
    '#caption_bottom' => t('Assets found: @total', array('@total' => $item_count_total))
  );

  return $form;
}

/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_asset_execute() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // What to sort on.
  $sort = $session['sort'];
  $order = $session['order'];
  $limit = $session['limit'];
  $page = $session['page'];
  $op = $session['op'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Last modified'):
      $order = mediamosa_asset_db::VIDEOTIMESTAMP;
      break;

    case t('Title'):
    default:
      $order = 'title';
      break;
  }

  $params = array();

  $cqls = array();

  switch ($op) {
    case t('Apply filter'):
    default:
      $cql_session = _mediamosa_browse_cql_build($session);
      if (!empty($cql_session)) {
        $cqls[] = '(' . $cql_session . ')';
      }

      $coll_id = isset($session['coll_id']) ? $session['coll_id'] : 0;
      if ($coll_id) {
        $cqls[] = strtr('AND coll_id == "^@coll_id^"', array('@coll_id' => $coll_id));
      }
      break;

    case t('Apply CQL filter'):
      $cqls[] = $session['cql'];
      break;
  }

  // Add sort.
  $cqls[] = strtr('sortBy @order/@sort', array('@order' => $order, '@sort' => drupal_strtolower($sort) == 'asc' ? 'ascending' : 'descending'));

  // Add params.
  $params['cql'] = implode(' ', $cqls);
  $params['limit'] = $limit;
  $params['offset'] = $page * $limit;

  // Do the rest call.
  return mediamosa_response_connector::static_do_restcall_drupal('asset', 'GET', $params);
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_asset_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] : array();

  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  foreach (array('operator', 'coll_id', 'filters', 'cql', 'op') as $param) {
    if (!empty($filter_form_state['input'][$param])) {
      $session[$param] = $filter_form_state['input'][$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Title'),
    'sort' => 'asc',
    'page' => 0,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
    'op' => t('Apply filter'),
  );

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] = $session;

  return $session;
}

/**
 * View the details of an asset.
 */
function mediamosa_maintenance_browse_asset_details($asset_id) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administer'), 'admin');
  $breadcrumb[] = l(t('MediaMosa 2'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Asset browser'), 'admin/mediamosa/browse/asset');
  drupal_set_breadcrumb($breadcrumb);

  // Get the asset.
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('asset/@asset_id', array('@asset_id' => rawurlencode($asset_id))) , 'GET');

  if ($response['header']['request_result_id'] != 601) {
    return '<h1>Asset not found</h1>';
  }

  $asset = reset($response['items']);

  $variables = array(
    'description' => t('Details for this asset are shown below.'),
    'properties' => array(
      t('App ID') => $asset[mediamosa_asset_db::APP_ID],
      t('Owner ID') => $asset[mediamosa_asset_db::OWNER_ID],
      t('Group ID') => $asset[mediamosa_asset_db::GROUP_ID],
    ),
    'metadata_description' => t('Metadata properties for the asset are shown below.'),
    'metadata' => array(),
  );

  foreach (array('dublin_core', 'qualified_dublin_core', 'czp', 'app_' . $asset[mediamosa_asset_db::APP_ID]) as $metadata_group) {
    if (empty($asset[$metadata_group])) {
      continue;
    }

    foreach ($asset[$metadata_group] as $name => $value) {
      if (empty($value)) {
        continue;
      }

      $variables['metadata'][t($name)] = $value;
    }
  }


  $build['properties'] = theme('mediamosa_maintenance_browse_properties', $variables);

  return implode('', $build);
}



/*
 * Single asset details.

function mediamosa_maintenance_browse_asset_details($asset_id) {
  $response = _mediamosa_restcall('asset/' . $asset_id, 'GET', array('is_app_admin' => 'true'));
  if (!$response) {
    return '';
  }

  $output = '';

  $asset = (array)$response->items->item;

  // Asset attributes
  $rows = array();
  foreach ($asset as $key => $value) {
    if (in_array($key, array('@attributes', 'czp', 'dublin_core', 'qualified_dublin_core'))) {
      continue;
    }
    if (trim($value) != '') {
      $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
      $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
    }
  }

  // Add the table to the output
  $output .= t('<h3>Asset attributes</h3>');
  $output .= theme('table', array('rows' => $rows));

  // Metadata
  foreach (array(
    'czp' => t('<h3>CZP Metadata</h3>'),
    'dublin_core' => t('<h3>Dublin Core Metadata</h3>'),
    'qualified_dublin_core' => t('<h3>Qualified Dublin Core Metadata</h3>'),
  ) as $subject => $title) {
    // Construct the table rows
    $rows = array();
    if (isset($asset[$subject])) {
      foreach ($asset[$subject] as $key => $value) {
        if (trim($value) != '') {
          $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
          $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
        }
      }
    }

    // Add the table to the output
    $output .= $title;
    $output .= theme('table', array('rows' => $rows));
  }

  // Mediafiles
  foreach ($response->items->item->xpath('mediafiles/mediafile') as $mediafile) {
    // Construct the table rows
    $rows = array();
    foreach ((array)$mediafile as $key => $value) {
      if (in_array($key, array('@attributes'))) {
        continue;
      }
      if (trim($value) != '') {
        $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
        $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
      }
    }

    // Add the table to the output
    $output .= t('<h3>Mediafile: @mediafile_id</h3>', array('@mediafile_id' => $mediafile->mediafile_id));
    $output .= theme('table', array('rows' => $rows));
  }

  return $output;
}

function _mediamosa_maintenance_browse_asset_get_all_assets($app_id) {
  $app_ids = explode(' ', $app_id);

  $response = _mediamosa_restcall('asset', 'GET', array('app_id' => $app_ids));

  if (!$response) {
    return array();
  }

  $assets = array();
  foreach ($response->xpath('//items/item') as $asset) {
    $title = trim((string)$asset->title);
    $assets[(string)$asset->coll_id] = ($title != '') ? $title : t('<title not set>');
  }

  return $assets;
}

/**
 * Helper function for the asset filter form

function _mediamosa_maintenance_browse_asset_list_filter() {
  $search_terms = array(
    'asset_info' => array(
      'asset_id' => t('Asset id'),
      'owner_id' => t('Owner id'),
      'group_id' => t('Group id'),
      'reference_id' => t('Reference id'),
      'batch_id' => t('Batch id'),
      'app_id' => t('App id'),
      //'provider_id' => t('provider_id'),
      'videotimestamp' => t('Video timestamp'),
      'videotimestampmodified' => t('Video timestamp modified'),
      //'play_restriction_start' => t('play_restriction_start'),
      //'play_restriction_end' => t('play_restriction_end'),
    ),
    'dublin_core' => array(
      'type' => t('Type'),
      'format' => t('Format'),
      'language' => t('Language'),
      'title' => t('Title'),
      'creator' => t('Creator'),
      'publisher' => t('Publisher'),
      'subject' => t('Subject'),
      'description' => t('Description'),
      'contributor' => t('Contributor'),
      'date' => t('Date'),
      'identifier' => t('Identifier'),
      'source' => t('Source'),
      'relation' => t('Relation'),
      'coverage_temporal' => t('Coverage temporal'),
      'coverage_spatial' => t('Coverage spatial'),
      'rights' => t('Rights'),
    ),
    'qualified_dublin_core' => array(
      'title_alternative' => t('Title alternative'),
      'description_abstract' => t('Description abstract'),
      'created' => t('Created'),
      'issued' => t('Issued'),
      'hasformat' => t('Has format'),
      'isformatof' => t('Is format of'),
      'format_medium' => t('Format medium'),
      'format_extent' => t('Format extent'),
      'license' => t('License'),
      'rightsholder' => t('Rightsholder'),
      'isreferencedby' => t('Is referenced by'),
    ),
  );
  foreach ($search_terms as $key => $value) {
    $search_terms[$key] = $value;
    asort($search_terms[$key]);
  }

  $search = array(
    0 => '', // Empty row
    'Asset information' => $search_terms['asset_info'],
    1 => '', // Empty row
    'Dublin Core' => $search_terms['dublin_core'],
    2 => '', // Empty row
    'Qualified Dublin Core' => $search_terms['qualified_dublin_core'],
  );

  return $search;
}

function _mediamosa_maintenance_browse_asset_get_cql_from_session() {
  $filters = $data = array();

  // Iterate all items stored in the form submit function
  foreach ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] as $key => $filter) {
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
  if (isset($filters['app_id'])) {
    $data['app_ids'] = $filters['app_id'];
    unset($filters['app_id']);
  }

  // Construct the cql
  $cql = array();
  foreach ($filters as $filter_group => $filter) {
    $cql[] = sprintf('%s == "%s"', $filter_group, implode(' ', $filter));
  }
  $data['cql'] = '('. implode(" AND ", $cql) .')';

  // Add the optional sort order
  if ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']) {
    $sortby = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']['sql'];
    $sortorder = ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']['sort'] == 'desc') ? 'descending' : 'ascending';
    $data['cql'] .= 'sortBy '. $sortby .'/'. $sortorder;
  }

  return $data;
}

/**
 * The browse form which also contains a list of browsable assets.

function mediamosa_maintenance_browse_asset_form() {
  if (isset($_GET['coll_id'])) {
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] = array('coll_id' => check_plain($_GET['coll_id']));
  }

  // Gather and prepare some variables.
  $form = array();
  $item_limit = 50;
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  // Gather all apps.
  $all_apps_raw = array();
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME, 'ASC') as $app) {
    $all_apps_raw[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::APP_NAME];
  }
  $all_apps = array(implode(' ', array_keys($all_apps_raw)) => t('<All applications>'));
  $all_apps += $all_apps_raw;

  $app_id = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['app_id'])) ? $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['app_id'] : reset(array_keys($all_apps));
  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['app_id'] = $app_id;
  $form['app_id'] = array(
    '#type' => 'select',
    '#options' => $all_apps,
    '#title' => t('Application'),
    '#default_value' => $app_id,
  );

  $all_assets_raw = _mediamosa_maintenance_browse_asset_get_all_assets($app_id);

  if (count($all_assets_raw)) {
    $all_assets = array_merge(array(0 => ''), $all_assets_raw);
  }
  else {
    $all_assets = array(0 => t('No assets found...'));
  }

  $coll_id = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'])) ? $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'] : 0;
  $form['coll_id'] = array(
    '#type' => 'select',
    '#options' => $all_assets,
    '#title' => t('Collection'),
    '#default_value' => $coll_id,
  );

  // Get the current filter count
  $filter_count = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'])) ? count($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters']) : 0;

  // Add one empty filter item
  $filter_count++;

  // Get the maximum number of filters
  $filter_max = variable_get('mediamosa_maintenance_browse_filter_max', 25);

  // Load and set all javascript related items
  drupal_add_js(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenance_browse.js');
  drupal_add_js(
    array(
      'mediamosa_maintenance_browse' => array(
        'filter_count' => $filter_count,
        'filter_max' => $filter_max,
      ),
    ),
    'setting'
  );

  $filter_types = _mediamosa_maintenance_browse_asset_list_filter();
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
      '#prefix' => '<div id="mediamosa-browse-asset-filter-'. $i .'" style="display: '. $display .'" class="mediamosa-browse-asset-filter'. $extra_class .'">',
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
    if (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'][($i - 1)])) {
      $info = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'][($i - 1)];
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
    '#value' => t('Add filter item'),
    '#suffix' => '<a href="#" id="edit-clear">Clear form</a>',
  );

  // Only show results on GET requests to prevent Drupal from fetching the results on form validation.
  if ($_SERVER['REQUEST_METHOD'] != 'POST') {

    $header = array(
      array('field' => 'title', 'data' => t('Title')),
      array('field' => 'app_id', 'data' => t('App ID')),
      array('field' => 'videotimestampmodified', 'data' => t('Last modified')),
    );
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort'] = (isset($_GET['order'])) ? tablesort_init($header) : FALSE;

    // Fetch the assets
    $cql_data = _mediamosa_maintenance_browse_asset_get_cql_from_session();
    $data = array(
      'limit' => $item_limit,
      'offset' => $offset,
      'cql' => $cql_data['cql'],
    );

    // Add app_id's
    if (isset($cql_data['app_ids'])) {
      $data['app_id'] = $cql_data['app_ids'];
    }

    $response = _mediamosa_restcall('asset', 'GET', $data);
    if ($response) {
      // Extract the item count.
      $item_count = (int)$response->header->item_count;
      $item_count_total = (int)$response->header->item_count_total;
    }

    // Return the form if no assets were found.
    if (!$response || !$item_count_total) {
      $form['results'] = array(
        '#markup' => '<p>'. t('No assets found...') .'</p>',
      );
      return $form;
    }

    // Correct the item limit if the count is less than it.
    if ($item_count_total < $item_limit) {
      $item_limit = $item_count_total;
    }
    drupal_set_title(t('Showing assets @from-@to of @total', array(
      '@from' => $offset + 1,
      '@to' => $offset + $item_count,
      '@total' => $item_count_total,
    )));

    $rows = array();
    foreach ($response->xpath('//items/item') as $asset) {
      $title = (isset($asset->dublin_core->title) && trim((string)$asset->dublin_core->title) != '') ? $asset->dublin_core->title : t('<title not set>');
      $rows[] = array(
        l($title, 'admin/mediamosa/browse/asset/' . $asset->asset_id),
        $asset->app_id,
        $asset->videotimestampmodified,
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

function mediamosa_maintenance_browse_asset_form_submit($form, $form_state) {
  $filters = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'filter_type_') === 0 && !is_numeric($value)) {
      $id = str_replace('filter_type_', '', $key);
      if (trim($form_state['values']['filter_value_'. $id]) != '') {
        $filters[] = array($value => trim($form_state['values']['filter_value_'. $id]));
      }
    }
  }

  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] = array(
    'filters' => $filters,
    'app_id' => $form_state['values']['app_id'],
  );

  if ($form_state['values']['coll_id'] != 0) {
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'] = $form_state['values']['coll_id'];
  }
}
*/