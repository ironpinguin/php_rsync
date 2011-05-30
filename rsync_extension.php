<?php
/**
 * This is a placeholder for the php_rsync extension where is a wrapper
 * extension for the librsync library
 */

/**
 * Completed successfully.
 */
define("RSYNC_DONE",  0);
/**
 * Blocked waiting for more data.
 */
define("RSYNC_BLOCKED",  1);
/**
 * Not yet finished or blocked. This value should never be returned to
 * the caller.
 */
define("RSYNC_RUNNING", 2);
/**
 * Test neither passed or failed.
 */
define("RSYNC_TEST_SKIPPED", 77);
/**
 * Error in file or network IO.
 */
define("RSYNC_IO_ERROR", 100);
/**
 * Command line syntax error.
 */
define("RSYNC_SYNTAX_ERROR", 101);
/**
 * Out of memory.
 */
define("RSYNC_MEM_ERROR", 102);
/**
 * End of input file, possibly unexpected.
 */
define("RSYNC_INPUT_ENDED", 103);
/**
 * Bad magic number at start of stream. Probably not a librsync file, or
 * possibly the wrong kind of file or from an incompatible library version.
 */
define("RSYNC_BAD_MAGIC", 104);
/**
 * Author is lazy.
 */
define("RSYNC_UNIMPLEMENTED", 105);
/**
 * Unbelievable value in stream.
 */
define("RSYNC_CORRUPT", 106);
/**
 * Probably a library bug.
 */
define("RSYNC_INTERNAL_ERROR", 107);
/**
 * Bad value passed in to library, probably an application bug.
 */
define("RSYNC_PARAM_ERROR", 108);

/**
 * System is unusable
 */
define("RSYNC_LOG_EMERG", 0);
/**
 * Action must be taken immediately
 */
define("RSYNC_LOG_ALERT", 1);
/**
 * Critical conditions
 */
define("RSYNC_LOG_CRIT", 2);
/**
 * Error conditions
 */
define("RSYNC_LOG_ERR", 3);
/**
 * Warning conditions
 */
define("RSYNC_LOG_WARNING", 4);
/**
 * Normal but significant condition
 */
define("RSYNC_LOG_NOTICE", 5);
/**
 * Informational
 */
define("RSYNC_LOG_INFO", 6);
/**
 * Debug-level messages
 */
define("RSYNC_LOG_DEBUG", 7);

/**
 * The length of the md4 checksum
 */
define("RSYNC_MD4_LENGTH", 16);
/**
 * The default strong length for the signaturfile
 */
define("RSYNC_DEFAULT_STRONG_LEN", 8);
/**
 * The default block length for the signaturfile
 */
define("RSYNC_DEFAULT_BLOCK_LEN", 2048);

/**
 * Funtion to generate the signaturfile from the given file.
 * the signaturfile and file can be an string with the relative or absult path to the file
 * on the system or a open php stream.
 *
 * @param php_stream|string $file         Here give the filepath or a stream for the file to read.
 * @param php_stream|string $signaturfile Here give the filepath or a stream for the file to store.
 */
function rsync_generate_signature($file, $signaturfile) {}

/**
 * Function to patch a file with the given delta patch.
 * the file, patchfile and newfile can be a string or a open php stream
 *
 *
 * @param php_stream|string $file    String with the filepath or a open read php stream.
 * @param php_stream|string $patch   String with the filepath or a open read php stream.
 * @param php_stream|string $newfile String with the filepath or a open write php stream.
 */
function rsync_patch_file($file, $patch, $newfile) {}

/**
 * Function to generate a patch file from the given signaturfile and the local file.
 *
 * @param php_stream|string $signaturfile String with the filepath or a open read php stream.
 * @param php_stream|string $file         String with the filepath or a open read php stream.
 * @param php_stream|string $patchfile    String with the filepath or a open write php stream.
 */
function rsync_generate_delta($signaturfile, $file, $patchfile) {}

/**
 * Function to set the logging callback funktion/method
 * The callback function must have the parameter $level and $msg.
 * The $level ist on of the RSYNC_LOG_* constants and teh $msg
 * is the logging message
 * Example: loggingFunction(int $level, string $msg)
 *
 * @param string/array $function
 */
function rsync_set_log_callback($function) {}

/**
 * Function to get the english description for the returncode
 *
 * @param integer $returncode
 *
 * @return string
 */
function rsync_error($returncode = null) {}

/**
 * Function to set the trace log level
 *
 * @param integer $level
 */
function rsync_set_log_level($level) {}
