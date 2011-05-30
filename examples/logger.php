<?php
/**
 * Logger class to register the logger for
 * to give a central logging with callback
 * registration in the php_rsync extension.
 *
 * @author ironpinguin
 *
 */
class rsyncLogger
{
    /**
     * Log file.
     *
     * @var string
     */
    private $logfile = null;

    /**
     * Instance variable for the logger singelton.
     *
     * @var logger
     */
    private static $instance = null;

    /**
     * Static method to get the instance of the
     * logger singelton.
     *
     * @return logger
     */
    public static function getInstance()
    {
        if (self::$instance == null) {
            self::$instance = new rsyncLogger();
        }
        return self::$instance;
    }

    /**
     * Empty Constructor....
     *
     */
    private function __construct()
    {
    }

    /**
     * Setter for the logfile.
     *
     * @param string $logfile String with the logfile incl. path.
     */
    public function setLogfile($logfile)
    {
        $this->logfile = $logfile;
    }

    /**
     * Logging method where will be register as callback in the
     * php_rsync extension.
     *
     * @param int    $level   Logging level.
     * @param string $message Logging message.
     */
    public function log($level, $message)
    {
        $msg = $level." : ".$message."\n";
        $type = 3;
        if ($logfile == null) {
            $type = 0;
        }
        error_log($message, $type, $this->logfile);
    }
}
