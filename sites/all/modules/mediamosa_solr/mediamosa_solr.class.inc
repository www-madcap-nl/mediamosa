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
 * Our extended classes for Solr connections.
 */

class mediamosa_solr {

  // ------------------------------------------------------------------- Consts.
  // In seconds, how long to process the queue before we stop for next run.
  const TIME_TO_PROCESS_QUEUE = 55;
  // when having x number assets, then commit. Be careful with memory when
  // increasing.
  const COMMIT_BULK_PER = 25;
  // Only store 50 chars max for sorting text.
  const VAR_CHAR_SORT_LENGTH = 50;
  // Number of runs we remember the # index during run.
  const TAKE_AVERAGE_LAST_X_MIN = 5;

  const WHERE_AND = 'AND';
  const WHERE_OR = 'OR';

  // array name for search in where
  const CQL = 'cql';
  const APPS = 'apps';
  const MF = 'mf';
  const ASSET = 'asset';
  const NO_MF = 'no_mf';
  const COLLS = 'colls';
  const BATCH = 'batch';
  const USER_FAV = 'user_fav';
  const ORG = 'original';
  const IS_PROTECTED_FALSE = 'is_protected_false';
  const IS_PROTECTED_TRUE = 'is_protected_true';
  const AUT = 'aut';
  const AUT_NAME = 'name';
  const AUT_GRP = 'group';



  //  const ORDER_BY = 'order_by';
//  const ASSET_COLLECTION = 'asset_coll';
//  const COLLECTION = 'collection';

  // ---------------------------------------------------------------- Functions.
  static public function asset_search($parameters) {
//{!prefix f=asset_id}[iets]*
    mediamosa_watchdog::log_export($parameters);

    // Make sure its unique.
    $parameters['app_ids'] = array_unique($parameters['app_ids']);

    $solr_query = array();

    // Get the slaves based on the app ids for mediafiles.
    $slave_mediafiles = mediamosa_aut_app_master_slave::slave_get($parameters['app_ids'], mediamosa_aut::AUT_TYPE_MEDIAFILE);

    // Get the slaves based on the app ids for assets.
    $slave_assets = array(); // Off, not stable, mediamosa_aut_app_master_slave::slave_get($a_app_ids, mediamosa_aut::AUT_TYPE_ASSET);

    // Merge the slave app ids with my app ids.
    $app_ids_all = array_unique(array_merge($parameters['app_ids'], array_keys($slave_mediafiles), array_keys($slave_assets)));
    sort($app_ids_all);

    // Add app_ids.
    foreach ($app_ids_all as $app_id) {
      $solr_query[self::WHERE_AND][self::ASSET][self::APPS][self::WHERE_OR] = 'app_id:' . (int) $app_id;
    }

    // isprivate / unappropiate test.
    // Is outside the ACL check, else we would have problems with 'granted'.
    if (!$parameters['is_app_admin']) {
      if ($parameters['is_public_list'] && $parameters['aut_user_id']) {
        $solr_query[self::WHERE_AND][self::ASSET][self::AUT][self::WHERE_AND][] = strtr('(isprivate:FALSE AND (is_unappropriate:FALSE OR owner_id:"@owner_id"))', array('@owner_id' => mediamosa_solr_apache_solr_service::escapePhrase($parameters['aut_user_id'])));
      }
      elseif ($parameters['is_public_list']) {
        $solr_query[self::WHERE_AND][self::ASSET][self::AUT][self::WHERE_AND][] = '(isprivate:FALSE AND is_unappropriate:FALSE)'; // Must both be FALSE
      }
      elseif ($parameters['aut_user_id']) { // if provided, then we only have access to unappropate when owner.
        $solr_query[self::WHERE_AND][self::ASSET][self::AUT][self::WHERE_AND][] = strtr('(is_unappropriate:FALSE OR owner_id:"@owner_id")', array('@owner_id' => mediamosa_solr_apache_solr_service::escapePhrase($parameters['aut_user_id'])));
      }
      else {
        // No public list, no aut_user_id
        $solr_query[self::WHERE_AND][self::ASSET][self::AUT][self::WHERE_AND][] = 'is_unappropriate:FALSE'; // Ignore isprivate, is_unappropriate must be TRUE
      }
    }

    // Switch to hide assets that have no mediafiles
    if ($parameters['hide_empty_assets']) {
      // exclude empty assets
      $solr_query[self::WHERE_AND][self::ASSET][self::AUT][self::WHERE_AND][] = 'is_empty_asset:FALSE';
    }

    if (!empty($parameters['cql'])) {
mediamosa_debug::log($parameters['cql']);
      $result = mediamosa_solr_cql::parse_asset($parameters['cql'], $parameters['app_ids']);
mediamosa_debug::log_export($result);

      // We got something from cql?
      if (!empty($result['str_where'])) {
        $solr_query[self::WHERE_AND][self::CQL] = $result['str_where'];
      }

      $order_bys = reset($result['a_order_by']);
mediamosa_debug::log_export($order_bys);

      $order_by = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
      $order_direction = drupal_strtolower($order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION]);
      $order_type = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE];
      $order_table = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
      $is_metadata = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID] > 0;

      // Create the seach CQL.
      //$result_cql2sql = mediamosa_core_cql::parse_asset($cql, $a_app_ids);
    }

    // If coll_id is given, then search within the given collection(s)
    if (!empty($parameters['coll_id'])) {
      assert(is_array($parameters['coll_id']));

      foreach ($parameters['coll_id'] as $coll_id) {
//{!prefix f=asset_id}[iets]*
        // Not a app_admin, then rules apply.
        // ^coll_id-isprivate-is_unappropriate-owner_id
        // ^coll_id-owner_id
        if (!$parameters['is_app_admin']) {
          $isprivate_false_false = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~FALSE~FALSE^', array('@coll_id' => $coll_id)));
          $isprivate_false_true = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~FALSE~TRUE^', array('@coll_id' => $coll_id)));
          $is_unappropriate_false_false = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~FALSE~FALSE^', array('@coll_id' => $coll_id)));
          $is_unappropriate_true_false = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~TRUE~FALSE^', array('@coll_id' => $coll_id)));
          $owner = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~@owner^', array('@coll_id' => $coll_id, '@owner' => mediamosa_solr_apache_solr_service::filterDelimeters($parameters['aut_user_id']))));

          if ($parameters['is_public_list'] && $parameters['aut_user_id']) {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('((coll_id:"@isprivate_1" OR coll_id:"@isprivate_2") AND (coll:"@unappropriate_1" OR coll:"@unappropriate_2" OR coll_id_owner:"@owner"))' , array('@isprivate_1' => $isprivate_false_false, '@isprivate_2' => $isprivate_false_true, '@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false, '@owner' => $owner));
            //("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id = '%s'))",
          }
          elseif ($parameters['is_public_list']) {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('((coll_id:"@isprivate_1" OR coll_id:"@isprivate_2") AND (coll:"@unappropriate_1" OR coll:"@unappropriate_2"))' , array('@isprivate_1' => $isprivate_false_false, '@isprivate_2' => $isprivate_false_true, '@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false));
            //"c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'";
          }
          elseif ($parameters['aut_user_id']) { // if provided, then we only have access to unappropate when owner.
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('(coll:"@unappropriate_1" OR coll:"@unappropriate_2" OR coll_id_owner:"@owner")' , array('@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false, '@owner' => $owner));
            //"(c.is_unappropriate = 'FALSE' OR c.owner_id = '%s')", mediamosa_solr_apache_solr_service::escapePhrase($parameters['aut_user_id']));
          }
          else {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('(coll:"@unappropriate_1" OR coll:"@unappropriate_2")' , array('@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false));
            //"c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
          }
        }
        else {
          $coll_id = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@coll_id~*^', array('@coll_id' => $coll_id)));
          $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR] = strtr('coll_id:"@coll_id"', array('@coll_id' => $coll_id));
        }
      }
    }

    // FTP Batch.
    if (!empty($parameters['batch_id'])) {
      $solr_query[self::WHERE_AND][self::BATCH] = strtr('batch_id:"@batch_id"', array('@batch_id' => mediamosa_solr_apache_solr_service::escapePhrase($parameters['batch_id'])));
    }

    if (!empty($parameters['fav_user_id'])) {
      foreach ($parameters['app_ids'] as $app_id) {
        $user_fav = mediamosa_solr_apache_solr_service::escapePhrase(strtr('^@app_id~@name^', array('@app_id' => $app_id, '@name' => mediamosa_solr_apache_solr_service::filterDelimeters($parameters['fav_user_id']))));
        $solr_query[self::WHERE_AND][self::USER_FAV][self::WHERE_OR][] = strtr('user_fav:"@user_fav"', array('@user_fav' => $user_fav));
      }
    }

    // AUT layer.
//mediamosa_aut::build_access_where($a_query, mediamosa_aut::AUT_TYPE_MEDIAFILE, NULL, $a_app_ids, $parameters['aut_user_id'], $parameters['aut_group_ids'], $aut_domain, $aut_realm, $a_slaves_mediafiles, FALSE, $do_master_slave);
              // build_access_where(&$a_query, $aut_type, $object_id, $mixed_app_id, $parameters['aut_user_id'], $parameters['aut_group_ids'], $aut_domain, $aut_realm, $slaves = NULL, $is_app_admin = FALSE, $do_master_slave_only = FALSE, $do_master_slave_null = FALSE, $app_id_table_prefix = 'a') {

    $build_access_app_ids = array_unique(array_merge($parameters['app_ids'], array_keys($slave_mediafiles)));
    sort($build_access_app_ids);

    // Get of this current user the aut_name/aut_group ids so we know his/her access.
    $aut_ids = mediamosa_aut::build_access($build_access_app_ids, $parameters['aut_user_id'], $parameters['aut_group_ids'], $parameters['aut_domain'], $parameters['aut_realm'], $slave_mediafiles);

    if (!is_null($parameters['aut_user_id']) || (!is_null($parameters['aut_group_ids']) && !empty($parameters['aut_group_ids'])) || count($aut_ids) || count($slave_mediafiles)) {
      // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps

      if (!$parameters['granted']) {
        // If $parameters['granted'] is TRUE, granted is TRUE, meaning we need to return all assets
        // even if we dont have access. This switch prevents returning all assets of the other
        // slave apps.
        foreach ($parameters['app_ids'] as $app_id) {
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::NO_MF][$app_id][self::WHERE_AND][] = 'app_id:' . (int) $app_id;
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::NO_MF][$app_id][self::WHERE_AND][] = 'mf:FALSE';
        }

        // If we have aut id for the master app, check them
        foreach ($parameters['app_ids'] as $app_id) {
          if (!empty($parameters['aut_user_id']) && !empty($parameters['aut_group_ids'])) {
          }
          else {
            if (!empty($parameters['aut_user_id'])) {
              $solr = array();
              $solr['app_id'] = $app_id;
              $solr['is_protected'] = 'FALSE';
              $solr['aut_user_id'] = mediamosa_solr_apache_solr_service::filterDelimeters($parameters['aut_user_id']);

              // Generate solr.
              $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][$app_id][self::WHERE_OR][] = '{!prefix f=mf_aut_obj_owner}' . mediamosa_solr_apache_solr_service::escapeForFunction(implode('~', $solr) . '~');
            }

            // group_id
            if (!empty($parameters['aut_group_ids'])) {
              $solr = array();
              $solr['app_id'] = $app_id;
              $solr['is_protected'] = 'FALSE';

              foreach ($parameters['aut_group_ids'] as $aut_group_id) {
                if (!empty($aut_group_id)) {
                  $solr['aut_group_id'] = mediamosa_solr_apache_solr_service::filterDelimeters($aut_group_id);

                  // Generate solr.
                  $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][$app_id][self::WHERE_OR][] = '{!prefix f=mf_aut_obj_group}' . mediamosa_solr_apache_solr_service::escapeForFunction(implode('~', $solr) . '~');
                }
              }
            }
          }
        }

        foreach ($parameters['app_ids'] as $app_id) {

          if (!empty($aut_ids[$app_id]['a_name'])) {
            $solr = array();
            $solr['app_id'] = $app_id;
            $solr['is_protected'] = 'TRUE';

            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['name'][self::WHERE_AND][] = "aut_obj.aut_type = 'NAME'";
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['name'][self::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $aut_ids[$app_id]['a_name']) . ")";
          }

          if (!empty($aut_ids[$app_id]['a_group']) || (isset($aut_ids['slaves']) && isset($aut_ids['slaves']['a_group'][$app_id]))) {
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['group'][self::WHERE_AND][] = "aut_obj.aut_type = 'GROUP'";
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['group'][self::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $aut_ids[$app_id]['a_group']) . ")";
          }

          if (!empty($parameters['aut_user_id'])) {
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]['user_id'] = "mf.owner_id = '" . mediamosa_db::escape_string($parameters['aut_user_id']) . "'";
          }

          // group_id
          if (!empty($parameters['aut_group_ids'])) {
            foreach ($parameters['aut_group_ids'] as $aut_group_id) {
              if (!empty($aut_group_id)) {
                $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND]["is_protected"][self::WHERE_OR]['group_id'][] = "mf.group_id = '" . mediamosa_db::escape_string($aut_group_id) . "'";
              }
            }
          }
        }

      }
      else {
        // When $parameters['granted'] is TRUE, we have always access to media of our own app_id(s)

        // Master app_id checks
        // Because the $parameters['app_ids'] is in the app_id selection of assets, this one is a bit useles...
        if (!empty($parameters['app_ids'])) {
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR]['master_app_id'][self::WHERE_AND]['ms'] = strtr('@app_id_table_prefix.app_id IN(@app_ids)', array('@app_id_table_prefix' => $app_id_table_prefix, '@app_ids' => implode(',', $parameters['app_ids'])));
        }
      }

      // possible slaves
      if (count($slave_mediafiles)) {
        $solr_query[mediamosa_db_query::A_JOIN]['aut_app_master_slave'] = mediamosa_aut_app_master_slave::join_aut_app_master_slave_get($aut_type);

        // If we need to check aut_object
        if (count($aut_ids)) {
          $solr_query[mediamosa_db_query::A_JOIN]['aut_object'] = mediamosa_aut_object::join_aut_object_get($aut_type);
        }

        foreach ($slave_mediafiles as $app_id_slave => $slave) {
          assert($app_id_slave);// should not happen
          // Only for app_ids that are present

          // Must be same app_id for slaves
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][] = $app_id_table_prefix . '.app_id=' . mediamosa_db::escape_string($app_id_slave);

          // Any case, a link to app must be present if we allow access to the MF
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][] = sprintf("aut_ms.app_id_master IN(%s)", implode(",", $parameters['app_ids']));
          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][] = sprintf("aut_ms.app_id_slave = %d", $app_id_slave);

          if ($parameters['granted']) {
            // If we only do master/slave, we include all that has been slaved and protected
            continue;
          }

          // 2 cases;
          // 1. The object is protected only app_id (is_protected == FALSE)
          //   then only access if app_id == slave_app_id
          // 2. The object is protected by other rights (is_protected == TRUE)
          //   then only access if app_id == slave_app_id and rights match with other rights

          $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_FALSE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_USER_USERGROUP);

          if (isset($aut_ids[$app_id_slave]) && count($aut_ids[$app_id_slave])) {
            if (count($aut_ids[$app_id_slave]['a_name'])) {
              $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);
            }

            // Even if we dont have access we still want it in our result when granted == TRUE, so skip it when granted is TRUE
            if (count($aut_ids[$app_id_slave]['a_name'])) {
              $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['name'][self::WHERE_AND][] = "aut_obj.aut_type='NAME'";
              $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['name'][self::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $aut_ids[$app_id_slave]['a_name']) . ")";
            }
          }

          if (isset($aut_ids['slaves_convert']) && isset($aut_ids['slaves_convert']['a_group'][$app_id_slave])) {
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][] = sprintf("(mf.is_protected='%s' OR mf.is_protected='%s')", mediamosa_aut::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_aut::MEDIAFILE_IS_PROTECTED_DOMAIN_REALM);

            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['group'][self::WHERE_AND][] = "aut_obj.aut_type='GROUP'";
            $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND]["is_protected"][self::WHERE_OR]["is_protected_TRUE"][self::WHERE_AND][self::WHERE_OR]['group'][self::WHERE_AND][] = "aut_obj.aut_id IN(" . implode(",", $aut_ids['slaves_convert']['a_group'][$app_id_slave]) . ")";
          }
        }
      }

    }
    else {
      $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::NO_MF] = 'mf:FALSE';

      if (!$parameters['granted']) { // Only where we have access on, this case when no protection.
        $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::APPS][self::WHERE_AND][] = 'mf_is_protected:FALSE';
      }

      // No M/S, only include assets from own app(s).
      foreach ($parameters['app_ids'] as $app_id) {
        $solr_query[self::WHERE_AND][self::AUT][self::WHERE_OR][self::APPS][self::WHERE_AND]['app_id'][self::WHERE_OR][] = 'app_id:' . (int) $app_id;
      }
    }


    // Do query.
    $query = self::where($solr_query);

    // Setup sorting.
    $query_params = array();
    if (!empty($order_by)) {
      $query_params['sort'] = $order_by . ($is_metadata ? ($order_direction == 'ASC' ? '_lo' : '_hi') : '') . ' ' . $order_direction;
    }

    $mediamosa_solr_apache_solr_response = mediamosa_solr::search($query, $parameters['offset'], $parameters['limit'], $query_params);
mediamosa_debug::log_export($mediamosa_solr_apache_solr_response);
    // Done.
    return array('asset_ids' => $mediamosa_solr_apache_solr_response->getAssetIds(), 'total_count' => $mediamosa_solr_apache_solr_response->getNumFound());
  }

  /**
   * Instant update on metadata?
   */
  static public function settingInstantlyMetadata() {
    return variable_get('mediamosa_solr_metadata_instantly', TRUE);
  }


  /**
   * Instant update on asset update?
   */
  static public function settingInstantlyAssetCreate() {
    return variable_get('mediamosa_solr_asset_create_instantly', TRUE);
  }

  /**
   * Instant update on asset update?
   */
  static public function settingInstantlyAssetUpdate() {
    return variable_get('mediamosa_solr_asset_update_instantly', TRUE);
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyAssetNormalize() {
    return self::settingInstantlyAssetUpdate(); // The same setting for now.
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyUserFav() {
    return variable_get('mediamosa_solr_user_fav_instantly', TRUE);
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyMasterSlave() {
    return variable_get('mediamosa_solr_master_slave_instantly', TRUE);
  }


  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL) {
    mediamosa_watchdog::log($message, $variables, $severity, 'Solr');
  }

  /**
   * Reindex with given asset_ids. Stores it in table, will be reindexed asap.
   *
   * @param string $asset_id
   */
  static public function queue_assets(array $asset_ids) {

    foreach ($asset_ids as $asset_id) {
      try {
        // If it fails because of existing primary key, then ignore.
        mediamosa_db::db_insert(mediamosa_solr_queue_db::TABLE_NAME)
          ->fields(array(
              mediamosa_solr_queue_db::ID => $asset_id,
              mediamosa_solr_queue_db::CREATED => mediamosa_datetime::db_current_timestamp_now()
          ))
          ->execute();
      }
      catch (PDOException $e) {
        // Ignore, we have no way to lock the table in drupal db layer, so no
        // select or update will work here.
      }
    }
  }

  /**
   * Remove the asset(s) from solr.
   *
   * @param string $asset_id
   */
  static public function delete_assets(array $asset_ids) {

    foreach ($asset_ids as $asset_id) {
      try {
        mediamosa_solr::delete_asset_document($asset_id, FALSE);
      }
      catch (Exception $e) {
        mediamosa_watchdog::log('Unable to delete asset from Solr.');
      }
    }

    // Commit all.
    self::commit();
  }

  /**
   * Insert asset data without delay.
   *
   * @param string $asset_id
   */
  static public function insert_assets(array $asset_ids) {

    $documents = array();
    foreach ($asset_ids as $asset_id) {
      try {
        // Create the document.
        $documents['documents'][] = self::create_asset_document($asset_id);
        $documents['asset_ids'][] = $asset_id;

        if (count($documents) == self::COMMIT_BULK_PER) {
          self::commit_documents($documents);
          $documents = array();
        }
      }
      catch (Exception $e) {
        // If one fails, then we can't fail the rest too.
       self::log($e->getMessage());
      }
    }

    // Commit if anything left.
    try {
      self::commit_documents($documents);
    }
    catch (Exception $e) {
      // Ignore.
      self::log($e->getMessage());
    }
  }

  /**
   * Return the count of the number of queued assets.
   */
  static public function queue_get_count() {
    return mediamosa_db::db_select(mediamosa_solr_queue_db::TABLE_NAME, 'sq')->countQuery()->execute()->fetchField();
  }

  /**
   * Triggered by REST call. Should not be run DURING the CRON!
   *
   * Will reindex the solr database as much as its allowed during this call.
   */
  static public function process_queue() {

    $mediamosa_solr_enable_queue = variable_get('mediamosa_solr_enable_queue', TRUE);
    if (empty($mediamosa_solr_enable_queue)) {
      // Always Optimize.
      self::optimize();

      return;
    }

    // Number of seconds the reindex is running max.
    $set_time_limit = variable_get('mediamosa_solr_reindex_time_limit', mediamosa_solr::TIME_TO_PROCESS_QUEUE);

    // Get possible current running reindex. Make sure its not running too.
    $run_last = variable_get('mediamosa_solr_queue_cron_last', NULL);

    // Make sure we don't run the queue process twice.
    // We will give it enough time to timeout.
    if ($run_last && round(REQUEST_TIME - $run_last) < (mediamosa_solr::TIME_TO_PROCESS_QUEUE * 3)) {
      self::log('Unexpected; Solr reindex was still / already running when cron was triggered.');
      return;
    }

    // If we get here then we can go.
    variable_set('mediamosa_solr_queue_cron_last', REQUEST_TIME);

    try {
      // 10 seconds more than we think we need.
      set_time_limit($set_time_limit + 10);

      // At where
      $end_at = REQUEST_TIME + $set_time_limit;

      $indexed = 0;

      // Get the now time, as long as we under end_at then continue.
      while (floor(microtime(TRUE)) < $end_at) {
        // Do our stuff until we are done or till we run out of time.
        $query = mediamosa_db::db_select(mediamosa_solr_queue_db::TABLE_NAME, 'sq');
        $query->fields('sq', array(mediamosa_solr_queue_db::ID));
        $query->range(0, self::COMMIT_BULK_PER);
        $query->orderBy(mediamosa_solr_queue_db::CREATED, 'ASC');
        $asset_ids = $query->execute()->fetchCol();

        // None left?
        if (empty($asset_ids)) {
          break;
        }

        $documents = array();
        foreach ($asset_ids as $asset_id) {
          // Create the document.
          $documents['documents'][] = self::create_asset_document($asset_id);
          $documents['asset_ids'][] = $asset_id;

          $indexed++;
        }

        // Commit will also remove the asset_ids from the queue.
        self::commit_documents($documents);
      }

      if ($indexed) {
        // Optimize.
        self::optimize();

        mediamosa_debug::log('Solr index run done, indexed @count assets', array('@count' => $indexed));
      }
    }
    catch (Exception $e) {
      // Exception, turn off that we are processing.
      variable_set('mediamosa_solr_queue_cron_last', NULL);
      variable_set('mediamosa_solr_queue_failures', $e->getMessage());
      throw $e;
    }

    // Done.
    variable_del('mediamosa_solr_queue_cron_last');
    variable_del('mediamosa_solr_queue_failures');

    // Save number of indexes for averages calc.
    $stats = variable_get('mediamosa_solr_queue_stats', array());
    $stats[] = $indexed;
    while (count($stats) > self::TAKE_AVERAGE_LAST_X_MIN) {
      array_shift($stats);
    }
    variable_set('mediamosa_solr_queue_stats', $stats);
  }

  /**
   * Will throw away index in Solr database, be careful with this function!
   *
   * @throws mediamosa_solr_exception
   */
  static public function purge_solr_index() {
    // Now purge, don't commit yet.
    self::deleteByQuery('*:*');

    // Commit.
    self::commit();
  }

  /**
   * Do search query.
   *
   * @param string $query
   *
   * @return
   *  Returns the Solr Response object.
   */
  static public function search($query, $offset = 0, $limit = 10, $params = array(), $method = 'GET') {
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    return new mediamosa_solr_apache_solr_response($mediamosa_apache_solr_service->search($query, $offset, $limit, $params, $method));
  }

  /**
   * Delete documents based on query. Don't forget to call commit.
   *
   * @param $query
   *
   * @return
   *  Returns the Solr Response object.
   */
  static public function deleteByQuery($query) {
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    return new mediamosa_solr_apache_solr_response($mediamosa_apache_solr_service->deleteByQuery($query));
  }

  /**
   * All must be commited.
   */
  static public function commit() {
    try {
      $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();
      $mediamosa_apache_solr_service->commit();
    }
    catch (Exception $e) {
      self::log($e->getMessage());
    }
  }

  /**
   * Delete documents of test data of the given app_ids.
   *
   * @param array $app_ids
   */
  static public function delete_simpletest_documents(array $app_ids = array()) {
    try {
      if (empty($app_ids)) {
        $solr_response = self::search('app_id:[' . (mediamosa_settings::APP_ID_MAX + 1) . ' TO *]');

        // How many to delete?
        $found = $solr_response->getNumFound();

        if ($found) {
          self::deleteByQuery('app_id:[' . (mediamosa_settings::APP_ID_MAX + 1) . ' TO *]');
          self::commit();
        }

        return $found;
      }
      else {
        $found_total = 0;

        // Delete metadata using the app_ids.
        foreach ($app_ids as $app_id) {
          $solr_response = self::search('app_id:' . $app_id);

          // How many to delete?
          $found = $solr_response->getNumFound();

          if ($found) {
            $found_total += $found;
            self::deleteByQuery('app_id:' . $app_id);
          }

        }

        // Commit.
        self::commit();

        return $found_total;
      }
    }
    catch (Exception $e) {
      // Cant react on exception, we might be in destructor.

      self::commit();
      return 0;
    }
  }

  /**
   * Delete one asset with data.
   */
  static public function delete_asset_document($asset_id, $commit = TRUE) {
    self::deleteByQuery('asset_id:' . mediamosa_solr_apache_solr_service::phrase($asset_id));

    if ($commit) {
      self::commit();
    }
  }

  /**
   * Insert one asset with data.
   */
  static public function create_asset_document($asset_id) {

    // Get all the data needed for indexing.
    $asset_for_index = mediamosa_asset::get_asset_for_index($asset_id);

    // Create empty document object asset;
    $document = new Apache_Solr_Document();

    // Asset data.
    $document->app_id = $asset_for_index['asset'][mediamosa_asset_db::APP_ID];
    $document->asset_id = $asset_for_index['asset'][mediamosa_asset_db::ID];

    $document->group_id = $asset_for_index['asset'][mediamosa_asset_db::GROUP_ID];
    $document->owner_id = $asset_for_index['asset'][mediamosa_asset_db::OWNER_ID];

    $document->viewed = $asset_for_index['asset'][mediamosa_asset_db::VIEWED];
    $document->played = $asset_for_index['asset'][mediamosa_asset_db::PLAYED];

    $asset_optionals = array(
      mediamosa_asset_db::PROVIDER_ID,
      mediamosa_asset_db::REFERENCE_ID,
      mediamosa_asset_db::MEDIAFILE_DURATION,
      mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE,
    );

    foreach ($asset_optionals as $optional) {
      if (!empty($asset_for_index['asset'][$optional])) {
        $document->{$optional} = $asset_for_index['asset'][$optional];
      }
    }

    // Get all technical metadata of all mediafiles.
    foreach ($asset_for_index['mediamosa_asset_mediafile_metadata'] as $mediafile_id => $mediafile_metadata) {
      // Skip stills.
      if ($mediafile_metadata['mediafile'][mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_FALSE) {
        continue;
      }

      // Mediafile.
      foreach (array(
        mediamosa_asset_mediafile_db::FILENAME,
        mediamosa_asset_mediafile_db::ID,
        mediamosa_asset_mediafile_db::URI,
        ) as $name) {
        if (!empty($mediafile_metadata['mediafile'][$name])) {
          $document->{$name} = $mediafile_metadata['mediafile'][$name];
        }
      }

      // Technical metadata.
      foreach (array(mediamosa_asset_mediafile_metadata::MIME_TYPE) as $name) {
        if (!empty($mediafile_metadata['metadata'][$name])) {
          $document->{$name} = $mediafile_metadata['metadata'][$name];
        }
      }
    }

    $asset_optionals_dates = array(
      mediamosa_asset_db::VIDEOTIMESTAMP,
      mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED,
      mediamosa_asset_db::CREATED,
      mediamosa_asset_db::CHANGED,
    );

    foreach ($asset_optionals_dates as $optional) {
      if (!empty($asset_for_index['asset'][$optional]) && $asset_for_index['asset'][$optional] != '0000-00-00 00:00:00') {
        $asset_for_index['asset'][$optional] = drupal_substr($asset_for_index['asset'][$optional], 0, 10) . 'T' . drupal_substr($asset_for_index['asset'][$optional], 11, 8) . 'Z';

        // Do not index invalid dates(!).
        if (!preg_match('/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z/', $asset_for_index['asset'][$optional])) {
          continue;
        }

        $document->{$optional} = $asset_for_index['asset'][$optional];
      }
    }

    // Optional.
    if (!empty($asset_for_index['batch_ids'])) {
      $document->batch_id = $asset_for_index['batch_ids'];
    }

    $document->is_empty_asset = $asset_for_index['asset'][mediamosa_asset_db::IS_EMPTY_ASSET] == mediamosa_asset_db::IS_EMPTY_ASSET_FALSE ? mediamosa_asset_db::IS_EMPTY_ASSET_FALSE : mediamosa_asset_db::IS_EMPTY_ASSET_TRUE;
    $document->isprivate = $asset_for_index['asset'][mediamosa_asset_db::ISPRIVATE] == mediamosa_asset_db::ISPRIVATE_TRUE ? mediamosa_asset_db::ISPRIVATE_TRUE : mediamosa_asset_db::ISPRIVATE_FALSE;
    $document->is_unappropriate = $asset_for_index['asset'][mediamosa_asset_db::IS_UNAPPROPRIATE] == mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE ? mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE : mediamosa_asset_db::IS_UNAPPROPRIATE_FALSE;
    $document->is_external = $asset_for_index['asset'][mediamosa_asset_db::IS_EXTERNAL] == mediamosa_asset_db::IS_EXTERNAL_TRUE ? mediamosa_asset_db::IS_EXTERNAL_TRUE : mediamosa_asset_db::IS_EXTERNAL_FALSE;

    // Collection ids. ^coll_id~isprivate~is_unappropriate^
    if (!empty($asset_for_index['collections'])) {
      foreach ($asset_for_index['collections'] as $coll_id => $collection) {
        $isprivate = $collection[mediamosa_collection_db::ISPRIVATE] == mediamosa_collection_db::ISPRIVATE_TRUE ? mediamosa_collection_db::ISPRIVATE_TRUE : mediamosa_collection_db::ISPRIVATE_FALSE;
        $is_unappropriate = $collection[mediamosa_collection_db::IS_UNAPPROPRIATE] == mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE ? mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE : mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE;
        $coll_ids[] = '^' . implode('~', array($coll_id, $isprivate, $is_unappropriate)) . '^';
        $coll_id_owners[] = '^' . implode('~', array($coll_id, mediamosa_solr_apache_solr_service::filterDelimeters($collection[mediamosa_collection_db::OWNER_ID]))) . '^';
      }

      $document->coll_id = $coll_ids;
      $document->coll_id_owner = $coll_id_owners;
    }

    // User fav.
    if (!empty($asset_for_index['user_favorites'])) {
      foreach ($asset_for_index['user_favorites'] as $user_favorite) {
        $user_favorites[] = '^' . implode('~', array($user_favorite[mediamosa_user_favorite_db::APP_ID], mediamosa_solr_apache_solr_service::filterDelimeters($user_favorite[mediamosa_user_favorite_db::NAME]))) . '^';
      }
      $document->user_fav = $user_favorites;
    }

    // Asset metadata.
    // [groupname][propertyname] = array('values' => array(value, value), 'type' => type).
    foreach ($asset_for_index['asset_metadata'] as $name => $metadata_group) {
      foreach ($metadata_group as $name => $metadata) {

        switch ($metadata['type']) {
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            // Sort from low to high.
            sort($metadata['values'], SORT_STRING);

            $document->{$name . '_vc'} = $metadata['values'];
            $document->{$name . '_svc_lo'} = substr(reset($metadata['values']), 0, self::VAR_CHAR_SORT_LENGTH);
            $document->{$name . '_svc_hi'} = substr(array_pop($metadata['values']), 0, self::VAR_CHAR_SORT_LENGTH);
            break;

          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            // Dates from mediamosa are not completely comp. with Solr.
            foreach ($metadata['values'] as $key => $date) {
              if ($date == '0000-00-00 00:00:00' || empty($date)) {
                continue;
              }

              $metadata['values'][$key] = drupal_substr($metadata['values'][$key], 0, 10) . 'T' . drupal_substr($metadata['values'][$key], 11, 8) . 'Z';

              // Do not index invalid dates(!).
              if (!preg_match('/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z/', $metadata['values'][$key])) {
                unset($metadata['values'][$key]);
              }
            }

            // Values left?
            if (empty($metadata['values'][$key])) {
              continue;
            }

            // Sort from low to high.
            sort($metadata['values'], SORT_STRING);

            $document->{$name . '_vd'} = $metadata['values'];
            $document->{$name . '_svd_lo'} = reset($metadata['values']);
            $document->{$name . '_svd_hi'} = array_pop($metadata['values']);
            break;

          case mediamosa_asset_metadata_property_db::TYPE_INT:
            // Sort from low to high.
            sort($metadata['values'], SORT_NUMERIC);

            $document->{$name . '_vi'} = $metadata['values'];
            $document->{$name . '_svi_lo'} = reset($metadata['values']);
            $document->{$name . '_svi_hi'} = array_pop($metadata['values']);
            break;

          default:
            throw new mediamosa_solr_exception('Unsupported metadata type detected!');
        }
      }
    }

    // Done.
    return $document;
  }

  /**
   * Commit the collected documents.
   *
   * @param array $documents
   */
  static public function commit_documents(array $documents, $optimize = FALSE) {
    if (empty($documents['documents'])) {
      return; // Nothing to commit.
    }

    // Get the service object.
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    if (count($documents['documents']) == 1) {
      $mediamosa_apache_solr_service->addDocument(reset($documents['documents']));
    }
    else {
      $mediamosa_apache_solr_service->addDocuments($documents['documents']);
    }

    // Commit the documents.
    $mediamosa_apache_solr_service->commit();

    // Optimize when indicated.
    if ($optimize) {
      // Optimize.
      $mediamosa_apache_solr_service->optimize();
    }

    // Remove from queue.
    mediamosa_db::db_delete(mediamosa_solr_queue_db::TABLE_NAME)->condition(mediamosa_solr_queue_db::ID, $documents['asset_ids'], 'IN')->execute();
  }

  /**
   * Do a optimize call.
   */
  static public function optimize() {
    // Get the service object.
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    // Optimize.
    $mediamosa_apache_solr_service->optimize();
  }

  /**
   * Create the WHERE syntax
   *
   * @param array $where
   * @return string
   */
  static public function where($where) {
    return implode(' AND ', self::where_2($where));
  }

  /**
   * self::where helper function
   *
   * @param array $where
   * @param string $l
   * @param string $r
   * @param string $glue
   * @return array
   */
  static public function where_2($where, $l = '', $r = '', $glue = ' AND ') {
    if (!is_array($where)) {
      return array($where);
    }

    $result = array();
    foreach ($where as $type => $a_statements) {
      if ($type == self::WHERE_AND) {
        $glue = " AND \n";
        $a_statements = self::where_2($a_statements, '(', ')', " AND \n");
        $result[] = $l . implode(" AND \n", $a_statements) . $r;
      }
      elseif ($type == self::WHERE_OR) {
        $a_statements = self::where_2($a_statements, '(', ')', ' OR ');
        $result[] = $l . implode(' OR ', $a_statements) . $r;
      }
      else {
        $result[] = implode($glue, self::where_2($a_statements, '(', ')', $glue));
      }
    }

    return $result;
  }

}
