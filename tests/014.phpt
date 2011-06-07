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
$notexists = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "not exists";
$nosense = "." . DIRECTORY_SEPARATOR . "tests" . DIRECTORY_SEPARATOR . "no sense";
 
try {
    rsync_generate_signature($notexists, $nosense);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_generate_signature($exists0, $notexists);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_generate_delta($notexists, $nosense, $nosense);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_generate_delta($sigfile, $notexists, $nosense);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_generate_delta($sigfile, $exists0, $notexists);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_patch_file($notexists, $nosense, $nosense);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

try {
    rsync_patch_file($sigfile, $notexists, $nosense);
} catch (RsyncFileIoException $e) {
    $out[] = $e->getMessage();
}

@unlink($notexists);
@unlink($nosense);

print implode("\n", $out) . "\n";
?>
--EXPECTF--
Could not open "./tests/not exists" for read: No such file or directory
Could not open "./tests/not exists" for read: No such file or directory
Could not open "./tests/not exists" for read: No such file or directory
Could not open "./tests/not exists" for read: No such file or directory
Could not open "./tests/not exists" for read: No such file or directory
