--TEST--
Test function rsync_patch_file with binary string as input.
--SKIPIF--
<?php if (!extension_loaded("rsync") || 
          !file_exists("tests/007test.png") || 
          !file_exists("tests/007test.patch")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e869400efb68a96bafc33bda9a940111';
$file = file_get_contents('tests/007test.png');
$patch = file_get_contents('tests/007test.patch');
$ret = rsync_patch_file($file, $patch, 'tests/007test2.png', 1);
$resultMD5 = md5_file('tests/007test2.png');

if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/007test2.png');
?>
--EXPECT--
SUCCESS
