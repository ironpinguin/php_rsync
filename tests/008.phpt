--TEST--
Test the rsync_generate_signatur function with stream as input and signaturfile output.
--SKIPIF--
<?php 
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e6048d9f1ed7367259270db93b43cf74';

$input = fopen('tests/008test.png','rb');
$sig = fopen('tests/008signatur.sig', 'wb');
$ret = rsync_generate_signature($input, $sig);
fclose($input);
fclose($sig);
$resultMD5 = md5_file('tests/008signatur.sig');
if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}
unlink('tests/008signatur.sig');
?>
--EXPECT--
SUCCESS
