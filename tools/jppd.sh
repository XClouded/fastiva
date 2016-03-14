BASEDIR=$(dirname $0) 
(cd $BASEDIR && java -classpath $BASEDIR/cod/Jpp3_RAW.jar com.wise.jpp.JppServer /work/fastiva 7777)

