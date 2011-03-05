--TEST--
Test RsyncInvalidArgumentException
--SKIPIF--
<?php
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php
$out = array();
$exists0 = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "001.phpt";
$sigfile = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "003signatur.sig";
 
try {
    rsync_generate_signature(42, "no sense");
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_signature($exists0, 42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta(42, "no sense", "no sense");
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, 42, "no sense");
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, $exists0, 42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file(42, "no sense", "no sense");
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file($sigfile, 42, "no sense");
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file($sigfile, $exists0, 42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_set_log_level(42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

print implode("\n", $out) . "\n";
?>
--EXPECTF--
"file" must be of the type string or stream
"signature file" must be of the type string or stream
"signature file" must be of the type string or stream
"file" must be of the type string or stream
"delta file" must be of the type string or stream
"file" must be of the type string or stream
"delta file" must be of the type string or stream
"new file" must be of the type string or stream
Invalid log level value