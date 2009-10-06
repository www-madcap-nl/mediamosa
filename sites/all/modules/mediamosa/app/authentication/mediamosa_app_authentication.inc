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
  * The APP authentication class.
  */

class mediamosa_app_authentication {
/*
function vpx_get_shared_key($username) {
  return db_result(db_query("SELECT shared_key FROM {client_applications} WHERE name = '%s' AND active = 1", $username));
}

function vpx_eua_ident_is_eua_active($username) {
  return (int)db_result(db_query("SELECT active FROM {client_applications} WHERE name = '%s'", $username));
}

function vpx_get_eua_record($username) {
  if (($resource = db_query("select * from client_applications where name='%s'", $username)) != FALSE) {
    if (($data = db_fetch_object($resource)) != FALSE) {
      return array(
        "id"   => $data->id,
        "name" => $data->name);
    }
  }

  return FALSE;
}

function dbus_cookie_sha1($command, $param) {
  if ($command == "AUTH") {
    if (vpx_eua_ident_is_eua_active($param) === FALSE) {
      session_destroy();
      return "ERROR invalid account";
    }
    if (vpx_eua_ident_is_eua_active($param) === 0) {
      session_destroy();
      return "ERROR inactive account";
    }
    $_SESSION["DBUS"]["username"] = $param;
    $_SESSION["DBUS"]["cookie_id"] = 0;
    $_SESSION["DBUS"]["challenge"] = md5(microtime() . VPX_EUA_AUTH_SECRET_PHRASE);
    return "DATA ". VPX_EUA_AUTH_CONTEXT ." ". $_SESSION["DBUS"]["cookie_id"] ." ". $_SESSION["DBUS"]["challenge"];
  }
  else if ($command == "DATA") {
    list($random, $response) = explode(" ", $param);
    if (($cookie = vpx_get_shared_key($_SESSION["DBUS"]["username"])) === FALSE) {
      return FALSE;
    }
    return $response == sha1($_SESSION["DBUS"]["challenge"] .":". $random .":". $cookie);
  }
  else {
    return "ERROR syntax error";
  }
}

function vpx_eua_dbus_protocol($line) {
  // Initialize identification.

  $auth_method = array(
    "DBUS_COOKIE_SHA1" => "dbus_cookie_sha1");

  if (isset($_SESSION["DBUS"]) == FALSE) {
    $_SESSION["DBUS"] = array(
      "mechanism"  => FALSE,
      "challenge"  => FALSE,
      "username"   => FALSE);
  }

  // Parse and handle command.

  list($command, $param) = explode(" ", trim($line), 2);
  $result = "ERROR unknown error";
  switch ($command) {
    case "AUTH":
      if ($param == "") {
        $result = "REJECTED ". implode(" ", array_keys($auth_method));
        break;
      }
      list($mechanism, $init) = explode(" ", $param, 2);
      if (isset($auth_method[$mechanism]) == FALSE) {
        $result = "ERROR unknown mechanism";
        break;
      }
      if (function_exists($auth_method[$mechanism]) == FALSE) {
        $result = "ERROR mechanism not implemented";
        break;
      }
      $_SESSION["DBUS"]["mechanism"] = $auth_method[$mechanism];
      $_SESSION["DBUS"]["username"] = FALSE;
      $_SESSION["DBUS"]["challenge"] = FALSE;
      $param = $init;
    case "DATA":
      if ($_SESSION["eua"]) {
        $result = "ERROR already identified";
        break;
      }
      if ($_SESSION["DBUS"]["mechanism"] == FALSE) {
        $result = "ERROR mechanism not specified";
        break;
      }
      $result = $_SESSION["DBUS"]["mechanism"]($command, $param);
      if ($result === TRUE) {
        if (($eua = vpx_get_eua_record($_SESSION["DBUS"]["username"])) != FALSE) {
          $_SESSION["eua"] = $eua;
          $result = "OK ". VPX_EUA_AUTH_SERVER_GUID;
        }
        else {
          $result = "ERROR database error";
        }
        unset($_SESSION["DBUS"]);
        break;
      }
      else if ($result === FALSE) {
        $_SESSION["DBUS"]["mechanism"] = FALSE;
        $_SESSION["DBUS"]["challenge"] = FALSE;
        $result = "REJECTED";
        break;
      }
      break;
    case "BEGIN":
      if ($_SESSION["eua"] == FALSE) {
        $result = "ERROR not identified";
      }
      else {
        $result = ERRORMESSAGE_OKAY;
      }
      break;
    case "CANCEL":
      unset($_SESSION["DBUS"]);
      unset($_SESSION["eua"]);
      $result = "REJECTED";
      break;
    case "ERROR":
      break;
    default:
      $result = "ERROR unknown command";
  }

  return $result;
}

function vpx_eua_ident($a_args) {
  $a_parameters = array(
    'dbus' => array(
      'value' => vpx_get_parameter_2($a_args["post"], 'dbus'),
      'type' => 'skip',
      'required' => TRUE
    )
  );

  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  $result = vpx_eua_dbus_protocol($a_parameters["dbus"]["value"]);

  if (substr($result, 0, 5) != "ERROR") {
    $response = new rest_response(array(
      "id"          => ERRORCODE_OKAY,
      "status"      => ERRORMESSAGE_OKAY,
      "description" => "DBUS authentication protocol"));
  } else {
    $response = new rest_response(array(
      "id"          => ERRORCODE_DBUS_PROTOCOL_ERROR,
      "status"      => ERRORMESSAGE_ERROR,
      "description" => "DBUS ".$result));
  }

  $response->add_item(array("dbus" => $result));

  return $response;
}

function vpx_eua_ident_madrest_call_authorized() {

  /**
   * all urls that start with 'external/' are now allowed from any IP address
   *
   * This currently include the VPX status and upload progress.
   *
   * RBL
   *
   */
/*
  // IP check on internal calls
  if (arg(0) === 'internal') {
    if ($ips = vpx_variable_get('vpx_allowed_internal_ips', FALSE)) {
      $ip_check = FALSE;
      $ips = explode('|', $ips);
      foreach ($ips as $ip) {
        if (strpos($_SERVER['REMOTE_ADDR'], $ip) === 0) { // if the remote IP address begins with $ip
          $ip_check = TRUE;
          break;
        }
      }
      if ($ip_check === FALSE) {
        return FALSE;
      }
    }
  }

  // Gebruiker is reeds ingelogd.
  if (isset($_SESSION["eua"])) {
    $_GET["app_id"] = $_SESSION["eua"]["id"];
    return;
  }

  // Vrij toegankelijke REST calls.
  $rules = array(
    array( # /check_cookie/*
      0 => "check_cookie"),
    array( # /login/*
      0 => "login"),
    array( # /mediafile/upload
      0 => "mediafile",
      1 => "upload"),
    array( # /video/$mediafile_id/upload - TODO: verwijder mij!!!
      0 => "video",
      2 => "upload"), // END TODO
    array( # /internal/*
      0 => "internal"),
    array( # /external/*
      0 => "external")
  );

  foreach ($rules as $rule) {
    $match = 0;
    foreach ($rule as $key => $value) {
      if (arg($key) == $value) {
        $match++;
      }
    }
    if ($match == count($rule)) {
      return;
    }
  }

  // Toegang weigeren
  return FALSE;
}
  */
}