<?php
class logger
{
    private $logfile = null;

    private static $instance = null;

    public static function getInstance()
    {
        if (self::$instance == null) {
            self::$instance = new logger();
        }
        return self::$instance;
    }

    private function __construct()
    {
    }

    public function setLogfile($logfile)
    {
        $this->logfile = $logfile;
    }

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