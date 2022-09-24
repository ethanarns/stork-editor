OUTPUT_DIR=_nds_unpack
REPACKED=repacked.nds

rm -r $OUTPUT_DIR

mkdir -p $OUTPUT_DIR

lib/ndstool -x rom.nds \
-9 $OUTPUT_DIR/arm9.bin -y9 $OUTPUT_DIR/y9.bin \
-d $OUTPUT_DIR/data -h $OUTPUT_DIR/header.bin \
-7 $OUTPUT_DIR/arm7.bin -y7 $OUTPUT_DIR/y7.bin \
-y $OUTPUT_DIR/overlay -t $OUTPUT_DIR/banner.bin