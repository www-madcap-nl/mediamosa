<?php
/**
 * Copyright (c) 2007 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 *
 * Permission is granted for use, copying, modification, distribution,
 * and distribution of modified versions of this work as long as the
 * above copyright notice is included
 */

require_once("settings.php");

/* 0 haal san/nas mountpoint op
 * 1 controleer parameters
 * 2 controleer ticket/file
 * 3 zet headers (standaard of uit file info?)
 * 4 stream de file
 * 5 verscheur de ticket?
 */

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

// controleer parameters (ticket/filename)
if (preg_match("@^([a-zA-Z0-9]+)/([^/]+)$@", $param, $matches) !== 1) {
  die("Invalid parameter");
}

// controleer de ticket
$ticket = $mountpoint ."/". DOWNLOAD_TICKET_LOCATION ."/". $matches[1];
if (!is_dir($ticket)) {
  die("Invalid download ticket");
}

// controleer de file
$filename = $matches[2];
$file = $ticket . '/' . urldecode($filename);
if (!file_exists($file)) {
  die("File not found");
}

// zet headers
header("Pragma: public");
header("Expires: 0");
header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
header("Cache-Control: private, false");
header("Content-Description: Download");
header("Content-Type: application/force-download"); // alternatief: application/octet-stream
header("Content-Length: ". filesize($file));
header("Content-Disposition: attachment; filename=\"". $filename . "\""); // force a save dialog.
header("Content-Transfer-Encoding: binary");

// stream de file en verscheur de ticket op het filesystem
if (@readfile($file) !== false) {
  @unlink($file);
  @rmdir($ticket);
}

exit();
