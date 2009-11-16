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
  * Authorization layer for assets, mediafiles functions.
  */

//class mediamosa_aut {
//function vpx_aut_name_get($app_id, $aut_type, $aut_name, $aut_group_id/*can be NULL, do not use 0*/) {
//  db_set_active('data');
//  $query = sprintf("SELECT * FROM {aut_name} WHERE app_id='%d' AND aut_type='%s' AND aut_group_id%s AND aut_name='%s'", $app_id, $aut_type, vpx_db_query_value($aut_group_id), db_escape_string($aut_name));
//  $db_result = db_query($query);
//  db_set_active();
//
//  return db_fetch_array($db_result);
//}
//
//function vpx_aut_name_delete($app_id, $aut_type, $aut_name, $aut_group_id/*can be NULL, do not use 0*/) {
//
//  $a_aut_name = vpx_aut_name_get($app_id, $aut_type, $aut_name, $aut_group_id);
//  if (!$a_aut_name) {
//    throw new vpx_exception_error_aut_name_not_found(array("@hostname" => $aut_name));
//  }
//  // Delete all links to this aut_name
//  vpx_aut_object_delete('NAME', $a_aut_name['aut_name_id']);
//
//  db_set_active('data');
//  $query = sprintf("DELETE FROM {aut_name} WHERE app_id=%d AND aut_type='%s' AND aut_group_id%s AND aut_name='%s'", $app_id, $aut_type, vpx_db_query_value($aut_group_id), db_escape_string($aut_name));
//  db_query($query);
//  db_set_active();
//}
//
//function vpx_aut_object_delete($aut_type, $aut_id) {
//  db_set_active('data');
//  db_query("DELETE FROM {aut_object} WHERE aut_type='%s' AND aut_id=%d", $aut_type, $aut_id);
//  db_set_active();
//}
//
//
///**
// * Creates a entry in the aut_name table.
// *
// * @param integer $app_id
// * @param string $s_name
// * @param string $aut_type
// * @param string $aut_group_id
// * @return integer
// *  return the last inserted ID or if already in DB, the ID of existing row
// */
//function vpx_acl_aut_name_create($app_id, $s_name, $aut_type, $aut_group_id = NULL) {
//
//  switch ($aut_type) {
//    case VPX_ACL_AUT_NAME_TYPE_APP: // no longer in the aut_name table
//      assert(0);
//      throw new vpx_exception_error_unexpected_error();
//    case VPX_ACL_AUT_NAME_TYPE_DOMAIN:
//    case VPX_ACL_AUT_NAME_TYPE_USER:
//    case VPX_ACL_AUT_NAME_TYPE_USER_GROUP:
//      $s_suffix = $s_name;
//      $s_prefix = NULL;
//      break;
//    case VPX_ACL_AUT_NAME_TYPE_REALM:
//      if (substr($s_name, "@") === FALSE) {
//        $s_suffix = "@" . $s_name;
//        $s_prefix = NULL;
//      }
//      else {
//        list($s_prefix, $s_suffix) = explode("@", $s_name, 2);
//        $s_suffix = "@" . $s_suffix;
//      }
//
//      if (trim($s_prefix) == "") {
//        $s_prefix = NULL;
//      }
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  $query = sprintf("SELECT aut_name_id FROM {aut_name} WHERE app_id=%d AND aut_type='%s' AND aut_group_id%s AND aut_name%s AND aut_prefix%s", $app_id, $aut_type, vpx_db_query_value($aut_group_id), vpx_db_query_value($s_suffix), vpx_db_query_value($s_prefix));
//  db_set_active('data');
//  $aut_id = db_result(db_query($query));
//  db_set_active();
//  if ($aut_id) {
//    return $aut_id;
//  }
//
//  // Create the name entry
//  db_set_active('data');
//  $query = sprintf("INSERT INTO {aut_name} SET app_id=%d, aut_type='%s', aut_group_id%s, aut_name%s, aut_prefix%s",
//    $app_id,
//    $aut_type,
//    vpx_db_query_value($aut_group_id, TRUE),
//    vpx_db_query_value($s_suffix, TRUE),
//    vpx_db_query_value($s_prefix, TRUE));
//
//  db_query($query);
//  $aut_id = vpx_db_get_last_id();
//  db_set_active();
//
//  // return the last inserted id
//  return $aut_id;
//}
//
//function vpx_acl_aut_group_create($app_id, $s_group, $aut_type) {
//
//  // Trim it
//  $s_group = trim($s_group);
//
//  switch ($aut_type) {
//    case VPX_ACL_AUT_NAME_TYPE_DOMAIN:
//      if (!vpx_acl_is_group_domain($s_group)) {
//        throw new vpx_exception_error_invalid_data_parameter(array("@parameter" => $s_group, "@data" => "domain group"));
//      }
//
//      break;
//    case VPX_ACL_AUT_NAME_TYPE_REALM:
//      if (!vpx_acl_is_group_realm($s_group)) {
//        throw new vpx_exception_error_invalid_data_parameter(array("@parameter" => $s_group, "@data" => "domain group"));
//      }
//
//      break;
//    default:
//      throw new vpx_exception_error_invalid_data_parameter(array("@parameter" => $s_group, "@data" => "domain group or realm group"));
//  }
//
//  db_set_active('data');
//  $a_object = db_fetch_array(db_query("SELECT * FROM {aut_group} WHERE app_id=%d AND aut_group_type='%s' AND aut_group_name='%s'", $app_id, $aut_type, db_escape_string($s_group)));
//
//  if ($a_object === FALSE) {
//    db_query("INSERT INTO {aut_group} SET app_id=%d, aut_group_type='%s', aut_group_name='%s'", $app_id, $aut_type, db_escape_string($s_group));
//    $aut_group_id = vpx_db_get_last_id();
//  }
//  else {
//    $aut_group_id = $a_object['aut_group_id'];
//  }
//
//  db_set_active();
//
//  return $aut_group_id;
//}
//
///**
// * Creates a link between object and aut_name
// *
// * @param string $aut_object_id
// * @param string $aut_type
// * @param string $aut_object_type
// * @param integer $aut_id
// * @param array $a_rights
// */
//function vpx_acl_link_to_object($aut_object_id, $aut_type, $aut_object_type, $aut_id, $a_rights = array(VPX_ACL_RIGHT_ACCESS)) {
//  // Test of link al bestaat?
//  db_set_active('data');
//  $db_result = db_query("SELECT * FROM {aut_object} WHERE aut_object_type='%s' AND aut_object_id='%s' AND aut_type='%s' AND aut_id=%d", $aut_object_type, $aut_object_id, $aut_type, $aut_id);
//  db_set_active();
//
//  if (db_fetch_array($db_result)) {
//    return;
//  }
//
//  // Insert it
//  db_set_active('data');
//  db_query("INSERT INTO {aut_object} SET aut_object_type='%s', aut_object_id='%s', aut_type='%s', aut_id=%d", $aut_object_type, $aut_object_id, $aut_type, $aut_id);
//  db_set_active();
//}
//
///**
// * Create a master - slave record.
// *
// * @param string $aut_object_id
// * @param string $vpx_acl_aut_type
// * @param integer $app_id
// * @param integer $app_id_slave
// * @param array $a_rights
// */
//function vpx_acl_aut_master_slave_create($aut_object_id, $vpx_acl_aut_type, $app_id, $app_id_slave, $a_rights = array(VPX_ACL_RIGHT_ACCESS)) {
//
//  // Make sure we dont allow that app ids of master and slave are the same.
//  if ($app_id == $app_id_slave) {
//    throw new vpx_exception_error(ERRORCODE_MASTERSLAVE_OWN_APP);
//  }
//
//  // @TODO: These queries will have chance to fail on a master-master DB situation.
//  db_set_active('data');
//  $db_result = db_query("SELECT * FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id='%s' AND app_master_id=%d AND app_slave_id=%d", $vpx_acl_aut_type, $aut_object_id, $app_id, $app_id_slave);
//  db_set_active();
//
//  if (db_fetch_array($db_result)) {
//    return;
//  }
//
//  // Insert it
//  db_set_active('data');
//  db_query("INSERT INTO {aut_app_master_slave} SET aut_object_type='%s', aut_object_id='%s', app_master_id=%d, app_slave_id=%d", $vpx_acl_aut_type, $aut_object_id, $app_id, $app_id_slave);
//  db_set_active();
//}
//
//
///**
// * Sets rights on the object
// *
// * @param integer $app_id
// * @param string $s_user_id
// * @param string $vpx_acl_aut_type
// * @param array $a_object
// *  Make sure its an object that is within the same app_id as the user
// *  Or else it will be denied access
// * @param array $a_app_ids
// *  Reserved, not supported yet.
// * @param array $a_aut_user_ids
// * @param array $a_aut_group_ids
// * @param array $a_aut_domains
// * @param array $a_aut_realms
// * @param array $a_rights
// * @param array $is_app_admin
// *  For future expansion
// */
//function vpx_acl_rights_set($app_id, $s_user_id, $vpx_acl_aut_type, $a_object, $a_app_ids, $a_aut_user_ids, $a_aut_group_ids, $a_aut_domains, $a_aut_realms, $b_replace = TRUE, $a_rights = array(VPX_ACL_RIGHT_ACCESS), $is_app_admin = FALSE) {
//  assert(count($a_rights));
//  assert($vpx_acl_aut_type == VPX_ACL_AUT_TYPE_MEDIAFILE || $vpx_acl_aut_type == VPX_ACL_AUT_TYPE_ASSET || $vpx_acl_aut_type == VPX_ACL_AUT_TYPE_COLLECTION); // media files for now
//
//  assert(!is_null($a_app_ids));
//  assert(!is_null($a_aut_user_ids));
//  assert(!is_null($a_aut_group_ids));
//  assert(!is_null($a_aut_domains));
//  assert(!is_null($a_aut_realms));
//
//  switch ($vpx_acl_aut_type) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["mediafile_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["asset_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_COLLECTION:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["coll_id"];
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  // Only owners (with the same app_id) can change access (for now)
//  vpx_acl_owner_check($app_id, $s_user_id, $s_object_app_id, $s_owner_id, $is_app_admin);
//
//  // Eerst all oude links weggooien
//  if ($b_replace) {
//    vpx_acl_rights_clear($app_id, $s_user_id, $vpx_acl_aut_type, $a_object, $is_app_admin);
//  }
//
//  $rights_set = $rights_set_user_usergroup = $rights_set_domain_realm = FALSE;
//
//  $a_result = array();
//
//  // Link ega access
//  // Master / Slave
//  // Only the owner can share his mediafile to another ega
//  // The other ega is then called 'master', and the mediafile shared is the slave.
//  foreach ($a_app_ids as $app_id_master) {
//    db_set_active();
//    $allow_masterslave = db_result(db_query("SELECT allow_masterslave FROM {client_applications} WHERE id = %d", $app_id_master));
//    db_set_active('data');
//    if ($allow_masterslave == 1) {
//      try {
//        vpx_acl_aut_master_slave_create($s_object_id, $vpx_acl_aut_type, $app_id_master, $app_id);
//
//        //$rights_set = TRUE; App_id is outside the other rights and must not clear any other rights
//
//        $a_result[] = array(
//          "type" => VPX_ACL_AUT_NAME_TYPE_APP,
//          "value" => $app_id_master,
//          "a_error" => FALSE,
//        );
//      }
//      catch (vpx_exception_error $e) {
//        $a_result[] = array(
//          "type" => VPX_ACL_AUT_NAME_TYPE_APP,
//          "value" => $app_id_master,
//          "a_error" => $e->vpx_exception_error_array_get(),
//        );
//      }
//    }
//    else {
//      //Throw an error that you are not allowed to create master/slave relations for this app.
//     throw new vpx_exception_error_masterslave_disallowed(array("@aut_id" => $app_id_master));
//    }
//  }
//
//  // Link users access
//  foreach ($a_aut_user_ids as $s_aut_user_id) {
//    try {
//      $aut_id = vpx_acl_aut_name_create($app_id, $s_aut_user_id, VPX_ACL_AUT_NAME_TYPE_USER);
//      assert($aut_id);
//      if (!$aut_id) {
//        throw new vpx_exception_error_unexpected_error();
//      }
//
//      db_set_active('data');
//      // Now link aut_name with object (using aut_object)
//      vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_NAME, $vpx_acl_aut_type, $aut_id);
//      db_set_active();
//
//      $rights_set = $rights_set_user_usergroup = TRUE;
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_USER,
//        "value" => $s_aut_user_id,
//        "a_error" => FALSE,
//      );
//    }
//    catch (vpx_exception_error $e) {
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_USER,
//        "value" => $s_aut_user_id,
//        "a_error" => $e->vpx_exception_error_array_get(),
//      );
//    }
//  }
//
//  foreach ($a_aut_group_ids as $s_aut_group_id) {
//    try {
//      $aut_id = vpx_acl_aut_name_create($app_id, $s_aut_group_id, VPX_ACL_AUT_NAME_TYPE_USER_GROUP);
//      assert($aut_id);
//      if (!$aut_id) {
//        throw new vpx_exception_error_unexpected_error();
//      }
//
//      db_set_active('data');
//      // Now link aut_name with object (using aut_object)
//      vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_NAME, $vpx_acl_aut_type, $aut_id);
//      db_set_active();
//
//      $rights_set = $rights_set_user_usergroup = TRUE;
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_USER_GROUP,
//        "value" => $s_aut_group_id,
//        "a_error" => FALSE,
//      );
//    }
//    catch (vpx_exception_error $e) {
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_USER_GROUP,
//        "value" => $s_aut_group_id,
//        "a_error" => $e->vpx_exception_error_array_get(),
//      );
//    }
//  }
//
//  foreach ($a_aut_domains as $s_aut_domain) {
//    try {
//      // Check if domain is group
//      if (vpx_acl_is_group_domain($s_aut_domain)) { //[...].group domain
//        $aut_id = vpx_acl_aut_group_create($app_id, $s_aut_domain, VPX_ACL_AUT_NAME_TYPE_DOMAIN);
//        assert($aut_id);
//        if (!$aut_id) {
//          throw new vpx_exception_error_unexpected_error();
//        }
//
//        db_set_active('data');
//        // Now link aut_name with object (using aut_object)
//        vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_GROUP, $vpx_acl_aut_type, $aut_id);
//        db_set_active();
//      }
//      else {
//        $aut_id = vpx_acl_aut_name_create($app_id, $s_aut_domain, VPX_ACL_AUT_NAME_TYPE_DOMAIN);
//        assert($aut_id);
//        if (!$aut_id) {
//          throw new vpx_exception_error_unexpected_error();
//        }
//
//        db_set_active('data');
//        // Now link aut_name with object (using aut_object)
//        vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_NAME, $vpx_acl_aut_type, $aut_id);
//        db_set_active();
//      }
//
//      $rights_set = $rights_set_domain_realm = TRUE;
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_DOMAIN,
//        "value" => $s_aut_domain,
//        "a_error" => FALSE,
//      );
//    }
//    catch (vpx_exception_error $e) {
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_DOMAIN,
//        "value" => $s_aut_domain,
//        "a_error" => $e->vpx_exception_error_array_get(),
//      );
//    }
//  }
//
//  foreach ($a_aut_realms as $s_aut_realm) {
//    try {
//      // They can also define groups here...
//      if (vpx_acl_is_group_realm($s_aut_realm)) { //@[...].group realm
//        $aut_id = vpx_acl_aut_group_create($app_id, $s_aut_realm, VPX_ACL_AUT_NAME_TYPE_REALM);
//        assert($aut_id);
//        if (!$aut_id) {
//          throw new vpx_exception_error_unexpected_error();
//        }
//
//        db_set_active('data');
//        // Now link aut_name with object (using aut_object)
//        vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_GROUP, $vpx_acl_aut_type, $aut_id);
//        db_set_active();
//      }
//      else {
//        $aut_id = vpx_acl_aut_name_create($app_id, $s_aut_realm, VPX_ACL_AUT_NAME_TYPE_REALM);
//        assert($aut_id);
//        if (!$aut_id) {
//          throw new vpx_exception_error_unexpected_error();
//        }
//
//        db_set_active('data');
//        // Now link aut_name with object (using aut_object)
//        vpx_acl_link_to_object($s_object_id, VPX_ACL_OBJ_LINK_TYPE_NAME, $vpx_acl_aut_type, $aut_id);
//        db_set_active();
//      }
//
//      $rights_set = $rights_set_domain_realm = TRUE;
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_REALM,
//        "value" => $s_aut_realm,
//        "a_error" => FALSE,
//      );
//    }
//    catch (vpx_exception_error $e) {
//      $a_result[] = array(
//        "type" => VPX_ACL_AUT_NAME_TYPE_REALM,
//        "value" => $s_aut_realm,
//        "a_error" => $e->vpx_exception_error_array_get(),
//      );
//    }
//  }
//
//  if ($rights_set) {
//    switch ($vpx_acl_aut_type) {
//      case VPX_ACL_AUT_TYPE_MEDIAFILE:
//        db_set_active('data');
//
//        if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
//          db_query("UPDATE {mediafile} SET is_protected='%s' WHERE mediafile_id='%s' ", VPX_ACL_MEDIAFILE_IS_PROTECTED_USER_USERGROUP, $s_object_id);
//        }
//        elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
//          db_query("UPDATE {mediafile} SET is_protected='%s' WHERE mediafile_id='%s' ", VPX_ACL_MEDIAFILE_IS_PROTECTED_DOMAIN_REALM, $s_object_id);
//        }
//        else {
//          db_query("UPDATE {mediafile} SET is_protected='TRUE' WHERE mediafile_id='%s' ", $s_object_id);
//        }
//
//        db_set_active();
//        break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//        db_set_active('data');
//
//        if (!$rights_set_domain_realm && $rights_set_user_usergroup) {
//          db_query("UPDATE {asset} SET is_protected='%s' WHERE asset_id='%s' ", VPX_ACL_MEDIAFILE_IS_PROTECTED_USER_USERGROUP, $s_object_id);
//        }
//        elseif ($rights_set_domain_realm && !$rights_set_user_usergroup) {
//          db_query("UPDATE {asset} SET is_protected='%s' WHERE asset_id='%s' ", VPX_ACL_MEDIAFILE_IS_PROTECTED_DOMAIN_REALM, $s_object_id);
//        }
//        else {
//          db_query("UPDATE {asset} SET is_protected='TRUE' WHERE asset_id='%s' ", $s_object_id);
//        }
//
//        db_set_active();
//      break; // nothing yet
//    case VPX_ACL_AUT_TYPE_COLLECTION:
//      break; // nothing yet
//    default:
//      throw new vpx_exception_error_unexpected_error();
//    }
//  }
//
//  return $a_result;
//}
//
//function vpx_acl_rights_clear($app_id, $s_user_id, $vpx_acl_aut_type, $a_object, $is_app_admin = FALSE) {
//  switch ($vpx_acl_aut_type) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["mediafile_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["asset_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_COLLECTION:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["coll_id"];
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  // Only owners (with the same app_id) can change access (for now)
//  vpx_acl_owner_check($app_id, $s_user_id, $s_object_app_id, $s_owner_id, $is_app_admin);
//
//  switch ($vpx_acl_aut_type) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      db_set_active('data');
//      db_query("UPDATE {mediafile} SET is_protected='FALSE' WHERE mediafile_id='%s' ", $s_object_id);
//      db_set_active();
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      db_set_active('data');
//      db_query("UPDATE {asset} SET is_protected='FALSE' WHERE asset_id='%s' ", $s_object_id);
//      db_set_active();
//      break;// no special stuff here
//    case VPX_ACL_AUT_TYPE_COLLECTION:
//      break;// no special stuff here
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  // Verwijder de links tussen object en de aut_name/aut_group tabel
//  // Hierdoor verwijder je alle rechten.
//  // Verwijder ook alle master / slave links
//  db_set_active('data');
//  db_query("DELETE FROM {aut_object} WHERE aut_object_type='%s' AND aut_object_id='%s'", $vpx_acl_aut_type, $s_object_id);
//
//  // Verwijder master / slave link, alleen orginele app_id kan deze verwijderen
//  // Wij zijn altijd slave en slaves kunnen zich slave maken of verwijderen (moet owner zijn van object)
//  db_query("DELETE FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id='%s' AND app_master_id > 0 AND app_slave_id=%d", $vpx_acl_aut_type, $s_object_id, $app_id);
//  db_set_active();
//
//  // Todo;
//  // Query die alle entries in aut_name / aut_group verwijderd die geen link hebben met een aut_object
//}
//
///**
// * Get the ACL rights of object.
// * Will check ownership of object.
// * On mediafiles it will allow slaves to get rights of object when not owner.
// *
// * @param unknown_type $app_id
// * @param unknown_type $s_user_id
// * @param unknown_type $vpx_acl_aut_type
// * @param unknown_type $a_object
// * @param unknown_type $is_app_admin
// * @return unknown
// */
//function vpx_acl_rights_get($app_id, $s_user_id, $vpx_acl_aut_type, $a_object, $is_app_admin = FALSE) {
//  switch ($vpx_acl_aut_type) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["mediafile_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["asset_id"];
//      break;
//    case VPX_ACL_AUT_TYPE_COLLECTION:
//      $s_owner_id = $a_object["owner_id"];
//      $s_object_app_id = $a_object["app_id"];
//      $s_object_id = $a_object["coll_id"];
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  switch ($vpx_acl_aut_type) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      // Need to be either owner + same app or slave of object.
//      vpx_acl_owner_check_ext($vpx_acl_aut_type, $s_object_id, $app_id, $s_user_id, $s_object_app_id, $s_owner_id, $is_app_admin);
//      break;
//
//    default:
//      // Only owners (with the same app_id) can get access (for now)
//      vpx_acl_owner_check($app_id, $s_user_id, $s_object_app_id, $s_owner_id, $is_app_admin);
//      break;
//  }
//
//  // Haal eerst de aut_id, zo we weten welke links er zijn
//  db_set_active('data');
//  $db_result = db_query("SELECT aut_id, aut_type FROM {aut_object} WHERE aut_object_type='%s' AND aut_object_id='%s'", $vpx_acl_aut_type, $s_object_id);
//  db_set_active();
//
//  $a_aut_ids = array();
//  while ($a_row = db_fetch_array($db_result)) {
//    $a_aut_ids[$a_row['aut_type']][] = $a_row['aut_id'];
//  }
//
//  $a_result = array(VPX_ACL_OBJ_LINK_TYPE_NAME => array(), VPX_ACL_OBJ_LINK_TYPE_GROUP => array(), VPX_ACL_OBJ_LINK_TYPE_APP => array());
//
//  $db_result_name = $db_result_group = FALSE;
//  db_set_active('data');
//  if (isset($a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_NAME]) && !empty($a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_NAME])) {
//    $db_result_name = db_query("SELECT * FROM {aut_name} WHERE aut_name_id IN(%s)", implode(",", $a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_NAME]));
//  }
//  if (isset($a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_GROUP]) && !empty($a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_GROUP])) {
//    $db_result_group = db_query("SELECT * FROM {aut_group} WHERE aut_group_id IN(%s)", implode(",", $a_aut_ids[VPX_ACL_OBJ_LINK_TYPE_GROUP]));
//  }
//
//  // Master / slave(s)
//  // Note / todo ;
//  // does the owner of the mediafile have the right to see other slave rights from other master app (not his app/ega)?
//  // For now we only allow rights that owner might have set (he cant set or remove app slave rights if he/she does not have the same master app
//  $db_result_master_slave = db_query("SELECT * FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id='%s' AND app_master_id > 0 AND app_slave_id=%d", $vpx_acl_aut_type, $s_object_id, $s_object_app_id);
//  db_set_active();
//
//  if ($db_result_name) {
//    while ($a_row = db_fetch_array($db_result_name)) {
//      $name = $a_row["aut_name"];
//      if ($a_row["aut_type"] == VPX_ACL_AUT_NAME_TYPE_REALM)  {
//        $name = $a_row["aut_prefix"] . $a_row["aut_name"];
//      }
//
//      $a_result[VPX_ACL_OBJ_LINK_TYPE_NAME][] = array("type" => "aut_" . strtolower($a_row["aut_type"]), "name" => $name);
//    }
//  }
//
//  if ($db_result_group) {
//    while ($a_row = db_fetch_array($db_result_group)) {
//      $a_result[VPX_ACL_OBJ_LINK_TYPE_GROUP][] = array("type" => "aut_" . strtolower($a_row["aut_group_type"]), "name" => $a_row["aut_group_name"]);
//    }
//  }
//
//  // Master / slave
//  // Master is de share, zodat master onze object mag zien
//  while ($a_master_slave = db_fetch_array($db_result_master_slave)) {
//    $a_result[VPX_ACL_OBJ_LINK_TYPE_APP][] = array("type" => "aut_app", "app_id_slave" => $a_master_slave["app_slave_id"], "app_id_master" => $a_master_slave["app_master_id"]);
//  }
//
//  return $a_result;
//}
//
///**
// * Haal de velden uit de mediafile die nodig zijn voor mogelijke acl functies
// *
// * @param string $mediafile_id
// * @param array $a_what
// * @return array
// */
//function vpx_acl_get_data_from_mediafile($mediafile_id, $a_what = array("app_id", "owner_id")) {
//  db_set_active('data');
//  $db_result = db_query("SELECT %s FROM {mediafile} WHERE mediafile_id = '%s'", implode(",", $a_what), $mediafile_id);
//  db_set_active();
//
//  return ($db_result ? db_fetch_array($db_result) : FALSE);
//}
//
///**
// * Copy the ACL rights, slave/master from one object to another, does not have to be the same object type
// * Remember that will add right(s), does not remove current rights.
// *
// * @param string $aut_object_type_source
// * @param string $aut_id_source
// * @param string $aut_object_type_dest
// * @param string $aut_id_dest
// */
//function vpx_acl_replace_obj_to_obj($aut_object_type_source, $aut_id_source, $aut_object_type_dest, $aut_id_dest) {
//  db_set_active('data');
//
//  // We replace, so remove any existing rights 1st.
//  db_query("DELETE FROM {aut_object} WHERE aut_object_type='%s' AND aut_object_id='%s'", $aut_object_type_dest, $aut_id_dest);
//  db_query("DELETE FROM {aut_app_master_slave} WHERE aut_object_type='%s' AND aut_object_id='%s'", $aut_object_type_dest, $aut_id_dest);
//
//  // Copy normal ACL rights
//  $db_result = db_query("SELECT aut_type, aut_id FROM {aut_object} WHERE aut_object_type = '%s' AND aut_object_id = '%s'", $aut_object_type_source, $aut_id_source);
//  while ($dbrow = db_fetch_array($db_result)) {
//    db_query("INSERT INTO {aut_object} SET aut_object_id = '%s', aut_type = '%s', aut_id = %d, aut_object_type = '%s'", $aut_id_dest, $dbrow['aut_type'], $dbrow['aut_id'], $aut_object_type_dest);
//  }
//
//  // Copy master/slave settings
//  $db_result = db_query("SELECT app_slave_id, app_master_id FROM {aut_app_master_slave} WHERE aut_object_type = '%s' AND aut_object_id = '%s'", $aut_object_type_source, $aut_id_source);
//  while ($dbrow = db_fetch_array($db_result)) {
//    db_query("INSERT INTO {aut_app_master_slave} SET aut_object_id = '%s', aut_object_type = '%s', app_slave_id = '%s', app_master_id='%s'", $aut_id_dest, $aut_object_type_dest, $dbrow['app_slave_id'], $dbrow['app_master_id']);
//  }
//
//  // Now copy the is_protected from source to dest.
//  switch ($aut_object_type_source) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      $is_protected = db_result(db_query("SELECT is_protected FROM {mediafile} WHERE mediafile_id = '%s'", $aut_id_source));
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      $is_protected = db_result(db_query("SELECT is_protected FROM {asset} WHERE asset_id = '%s'", $aut_id_source));
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  // Now update the new object with the copy is_protected.
//  switch ($aut_object_type_source) {
//    case VPX_ACL_AUT_TYPE_MEDIAFILE:
//      db_query("UPDATE {mediafile} SET is_protected = '%s' WHERE mediafile_id = '%s'", $is_protected, $aut_id_dest);
//      break;
//    case VPX_ACL_AUT_TYPE_ASSET:
//      db_query("UPDATE {asset} SET is_protected = '%s' WHERE asset_id = '%s'", $is_protected, $aut_id_dest);
//      break;
//    default:
//      throw new vpx_exception_error_unexpected_error();
//  }
//
//  db_set_active();
//}
//
///**
// * Copy the ACL rights from one mediafile to another mediafile
// *
// * @param string $mediafile_id_src
// * @param string $mediafile_id_dest
// */
//function vpx_acl_replace_mediafile_to_mediafile($mediafile_id_src, $mediafile_id_dest) {
//  return vpx_acl_replace_obj_to_obj(VPX_ACL_AUT_TYPE_MEDIAFILE, $mediafile_id_src, VPX_ACL_AUT_TYPE_MEDIAFILE, $mediafile_id_dest);
//}
//
//}
