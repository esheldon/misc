hadoop jar "$HADOOP_HOME/contrib/streaming/hadoop-streaming.jar" \
    -input "hdfs:///user/esheldon/lensing/scat/05/" \
    -output "hdfs:///user/esheldon/outputs/test-rm01-full" \
    -file sobjshear \
    -mapper sobjshear \
    -file redshear \
    -reducer redshear \
    -cmdenv CONFIG_URL="hdfs:///user/esheldon/test-config/test2.config"

