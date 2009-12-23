<?php
require_once("../sites/default/settings.php");

define("VPX_REST_SERVER", substr(REST_URL, 0, strlen(REST_URL)-1));
define("DOWNLOAD_TICKET_LOCATION", "download_links");
define("STILL_TICKET_LOCATION", "still_links");
