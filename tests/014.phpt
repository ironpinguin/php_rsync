--TEST--
Test RsyncFileIoException
--SKIPIF--
<?php
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php
$out = array();
$exists0 = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "001.phpt";
$sigfile = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "003signatur.sig";
 
try {
    rsync_generate_signature("not exists", "no sense");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_signature($exists0, "not exists");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta("not exists", "no sense", "no sense");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, "not exists", "no sense");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, $exists0, "not exists");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file("not exists", "no sense", "no sense");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file($sigfile, "not exists", "no sense");
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file($sigfile, $exists0, "not exists");
} catch (RsyncFileIoException $e) {
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