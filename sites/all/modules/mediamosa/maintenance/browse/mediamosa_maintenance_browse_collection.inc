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
  */

/**
 * Helper function for the collection filter form
 */
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
 */
function mediamosa_maintenance_browse_collection_form() {
  // Gather and prepare some variables.
  $form = array();
  $item_limit = 50;
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  // Gather all apps
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::NAME), mediamosa_app_db::NAME, 'ASC') as $app) {
    $all_apps_raw[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::NAME];
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
    '#value' => t('Add filter item'),
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
 */
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
