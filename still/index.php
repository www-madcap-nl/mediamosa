<?php
/**
 * Copyright (c) 2007 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 *
 * Permission is granted for use, copying, modification, distribution,
 * and distribution of modified versions of this work as long as the
 * above copyright notice is included
 */

require_once("../download/settings.php");

// vraag de san/nas mountpoint op
if (($response = @file_get_contents(VPX_REST_SERVER ."/internal/get_current_mount_point")) === false) {
  die("Unable to connect to VPX");
}
if (preg_match("@<current_mount_point>.*</current_mount_point>@", $response, $matches) !== 1) {
  die("Invalid mountpoint received");
}
$mountpoint = $matches[0];
$mountpoint = str_replace("<current_mount_point>", "", $mountpoint);
$mountpoint = str_replace("</current_mount_point>", "", $mountpoint);

$param = str_replace('\\', '', $_GET['param']);

//print $param;
//die;

// controleer parameters (ticket/filename)
if (preg_match("@^([a-zA-Z0-9]+)$@", $param, $matches) !== 1) {
  die("Invalid parameter");
}

// controleer de ticket
$ticket = $mountpoint ."/". STILL_TICKET_LOCATION ."/". $matches[1];
$file = $ticket;
if (!file_exists($file)) {
  die("File not found");
}

header('Content-Type: image/jpeg');

// stream de file en verscheur de ticket op het filesystem
if (@readfile($file) !== false) {
  @unlink($file);
}

exit();
