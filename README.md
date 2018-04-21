# tsdspacket

## Usage

Reading from the standard input, **tsdspacket** interprets UDP D-Star packets from tshark. For example

```
tshark -i lo -f "udp port 20010" -x | tsdspacket
```
will read the local DSPR-formated header and voice D-Star UDP packets between MMDVMHost and the D-Star gateway. Executed without pipe redirection, `tshark -i lo -f "udp port 2010 -x` will produce output like:

```
0000  00 00 00 00 00 00 00 00 00 00 00 00 08 00 45 00   ..............E.
0010  00 4d 41 47 40 00 40 11 fb 56 7f 00 00 01 7f 00   .MAG@.@..V......
0020  00 01 4e 2b 4e 2a 00 39 fe 4c 44 53 52 50 20 b1   ..N+N*.9.LDSRP .
0030  ef 00 00 00 00 4e 37 54 41 45 20 20 47 4e 37 54   .....N7TAE  GN7T
0040  41 45 20 20 43 51 4e 54 45 53 54 20 54 4e 37 54   AE  CQNTEST TN7T
0050  41 45 20 20 20 54 4f 4d 20 d3 52                  AE   TOM .R

0000  00 00 00 00 00 00 00 00 00 00 00 00 08 00 45 00   ..............E.
0010  00 31 41 48 40 00 40 11 fb 71 7f 00 00 01 7f 00   .1AH@.@..q......
0020  00 01 4e 2b 4e 2a 00 1d fe 30 44 53 52 50 21 b1   ..N+N*...0DSRP!.
0030  ef 00 00 ce cc 2e 31 e5 17 68 61 f0 55 2d 16      ......1..ha.U-.

0000  00 00 00 00 00 00 00 00 00 00 00 00 08 00 45 00   ..............E.
0010  00 31 41 4a 40 00 40 11 fb 6f 7f 00 00 01 7f 00   .1AJ@.@..o......
0020  00 01 4e 2b 4e 2a 00 1d fe 30 44 53 52 50 21 b1   ..N+N*...0DSRP!.
0030  ef 01 00 ba 49 2e 41 65 cc 53 67 a4 30 1e e6      ....I.Ae.Sg.0..
```

**tsdspacket** will read this input and produce the output like:

```
**** DSRP Header **** UDP Header: IP 127.0.0.1:20011 --> 127.0.0.1:20010
tag=20 id=EFB1 seq=00 flag=00 00 00 R2=N7TAE  G R1=N7TAE  C UR=QNTEST T MY=N7TAE   /TOM  checksum=52D3

**** DSRP Data **** UDP Header: IP 127.0.0.1:20011 --> 127.0.0.1:20010
tag=21 id=EFB1 seq=00 err=00 ambe=CECC2E31E5176861F0 Slow data=<SYNC> {0x55, 0x2d, 0x16}

**** DSRP Data **** UDP Header: IP 127.0.0.1:20011 --> 127.0.0.1:20010
tag=21 id=EFB1 seq=01 err=00 ambe=BA492E4165CC5367A4 Slow Data='@Qu'
```

This indicates that the packets were DSRP Header and Data packets.

Three different formats of D-Star Packets are read and interpreted:
o DSRP packets are internal header and data packets used by MMDVMHost, ircDDBGateway and QnetRelay.
o DSTR packets are internal header and data packets used by QnetGateway, QnetRelay and modems directly supported by QnetGateway, namely QnetDVAP and QnetDVRPTR.
o DSVT packets are external header and data packets used by all irc network clients, including ircDDBGateway and QnetGateway. It us also used by the QnetLink --> QnetGateway link, on the local UDP port 40000.

Other packets detected include DSRP and DSTR poll packets.

The output of each packet begins with the type of packet and the source and destination IP:port contained in the UDP header. Nearly all numerical parameters from the packets themselves are printed in hexidecimal format. The three bytes of slow data found in all data packets are descrambled before being printed and sync data frames are appropriately labelled, like in the example above. Of course the ambe portion of each data packet represents exactly 20 milliseconds of voice pcm data vocoded by the ambe vocoder found in all D-Star radios and digital voice dongles.

Of course, because **tsdspacket** reads from the standard input, you can redirect the output of tshark to a file and interpret the file later with input redirection:

```
tsdspacket < some_file
```

If you accidently connect tshark to a port that doesn't contain DSRP, DSTR or DSVT packets, you'll get a lot of "unknown packet type" messages.

