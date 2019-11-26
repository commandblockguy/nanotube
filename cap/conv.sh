d=$(date +%Y_%m_%d-%H:%M:%S)
mkdir $d
rm -r latest
ln -s $d latest
find . -name "*.8Xv" -exec convhex -b {} $d/{}.bin \;
rm *.8Xv
cd $d
cp ETHLOG.8Xv.bin log
cp LOGBKP.8xv.bin logbkp
find . -name "P*X.8Xv.bin" -print0 | sort -z | xargs -I filename -r0 od -Ax -tx1 -v "filename" | text2pcap - all.pcap
find . -name "P*TX.8Xv.bin" -print0 | sort -z | xargs -I filename -r0 od -Ax -tx1 -v "filename" | text2pcap - tx.pcap
find . -name "P*RX.8Xv.bin" -print0 | sort -z | xargs -I filename -r0 od -Ax -tx1 -v "filename" | text2pcap - rx.pcap

