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
$notexists = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "not exists";
$nosense = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "no sense";
 
try {
    rsync_generate_signature(42, $nosense);

} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_signature($exists0, 42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta(42, $nosense, $nosense);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, 42, $nosense);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_generate_delta($sigfile, $exists0, 42);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file(42, $nosense, $nosense);
} catch (RsyncInvalidArgumentException $e) {
    $out[] = $e->getMessage();
}

try {
    rsync_patch_file($sigfile, 42, $nosense);
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

@unlink("." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "no sense");
@unlink("no sense");
@unlink("." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "not exists");
@unlink("not exists");

print implode("\n", $out) . "\n";
?>
--EXPECTF--
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Expected string or stream, "42" was given
Invalid log level value
