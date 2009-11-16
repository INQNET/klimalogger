#!/usr/bin/python

# Example script for decoding TFA-Dostmann 30.3015 "Klima Logger" EEPROM dumps
#
# http://stud3.tuwien.ac.at/~e9725348/Projects/klimalogger/

import sys
import datetime
from operator import add,itemgetter,mul

def byte2nibble(byte):
    assert 0 <= byte <= 0xff
    l = byte & 0xf
    h = (byte & 0xf0) >> 4
    return [l,h]

def bytes2nibble(bytes):
    assert bytes
    return reduce(add, map(byte2nibble, bytes))

def nibbles2bytes(nibbles):
    assert nibbles
    return [ (h*16+l) for l,h in zip(nibbles[::2],nibbles[1::2]) ]

def parse_bcd(byte):
    l,h = byte2nibble(byte)
    if l>9 or h>9:
        return None
    return h*10+l

def parse_timestamp(data):
    tmp = MM,HH,dd,mm,yy = map(parse_bcd,data)
    return datetime.datetime(2000+yy,mm,dd,HH,MM)

def parse_H(nibbles):
    assert len(nibbles) == 2
    if all((x == 10) for x in nibbles):
        return None
    assert all((x < 10) for x in nibbles)

    return sum(map(mul,nibbles,[1,10]))

def parse_T(nibbles):
    assert len(nibbles) == 3
    if all((x == 10) for x in nibbles):
        return None

    assert all((x < 10) for x in nibbles)

    return sum(map(mul,nibbles,[1,10,100]))-300


def parse_file(fname):
    print "="*79

    data = open(fname,'rb').read()
    print "input file: %s (len = %d)" % (fname, len(data))
    #assert len(data) == 32*1024

    eof_marker = data.find("\x5a\x2f")
    if eof_marker < 0:
        raise RuntimeError("EOF Marker not found")

    data = map(ord, data)


    ts = bytes2nibble(data[2:6])

    dt = parse_timestamp(data[:2]+nibbles2bytes(ts[1:]))

    weekdays = dict((i+1,w) for i,w in enumerate("Mon Tue Wed Thu Fri Sat Sun".split()))
    print "timestamp:", dt, weekdays[ts[0]],ts[-1]

    timezones = dict([ (((0x100+i)&0xff),i) for i in range(-12,13) ])

    print "timezone: %+.2d00" % timezones[data[6]]

    assert data[7] == 0xff # unknown

    print "unread entries: %.2x%.2x" % (data[10],data[9])

    intervals = dict(enumerate([1,5,10,15,20,30,60,2*60,4*60,6*60,8*60,12*60,24*60]))

    buffer_overflowed = bool(data[0xb] & 0x04)

    print "buffer overflowed:", buffer_overflowed
    print "interval: %.2x (= %d-minute interval)" % (data[0x8],intervals[data[0x8]])
    print "flags:", bytes2nibble(data[0xB:0xD])

    sensor_cnt,tmp = byte2nibble(data[0xC])
    assert tmp == 0

    record_len = {
        0: 5+5,
        1: 5+5,
        2: 5+8,
        3: 5+10,
        4: 5+13,
        5: 5+15
    }[sensor_cnt]

    assert data[0xD] == 0xff # unknown
    assert data[0xE] == 0xff # unknown

    print "calibration data:"
    for i in xrange(6):
        print "  T%d %.2x%.2x, H%d %.2x" % (i,data[0xf+(2*i)],data[0xf+(2*i)+1],
                                            i,data[0xf+12+i])

    print "alarms:"
    for i in xrange(6):
        d = data[0x2d+(i*5):0x2d+5+(i*5)]
        d2 = bytes2nibble(d)

        assert all(d2[i] == 0 for i in [0,1,5,6])

        t_lo = parse_T(d2[7:10])
        t_hi = parse_T(d2[2:5])
        h_lo = parse_H(byte2nibble(data[0x22+(2*i)]))
        h_hi = parse_H(byte2nibble(data[0x21+(2*i)]))

        #print "  T%d" % i, " ".join(("%.2x" % x) for x in d), d2
        print "  T%d low %3d, high %3d;" % (i,t_lo,t_hi),
        print "H%d low %2d, high %2d" % (i,h_lo,h_hi)

    print "sensor ids:"," ".join(("%.2x" % x) for x in data[0x4b:0x50])

    assert (data[0x50] & 0xe0) == 0x00
    print "valid sensors:", ", ".join(str(i+1) for i in xrange(5) if (data[0x50] & (1<<i)))

    print "sensors logged:", sensor_cnt
    print "record len:", record_len

    print "eof marker at 0x%x" % eof_marker

    print "-"*79

    ############################################################################
    # and now parse the log records...

    def parse_record(rec_data, sensor_cnt):
        if rec_data[0] == 0xff:
            return None,[],[]

        ts = parse_timestamp(rec_data[:5])

        nibbles = bytes2nibble(rec_data[5:])

        T = [parse_T(nibbles[0:3])] 
        H = [parse_H(nibbles[6:8])]

        if sensor_cnt>0:
            T.append(parse_T(nibbles[3:6]))
            H.append(parse_H(nibbles[8:10]))

            for idx in xrange(1, sensor_cnt):
                sens_ofs = 5+(idx*5)
                T.append(parse_T(nibbles[sens_ofs:sens_ofs+3]))
                H.append(parse_H(nibbles[sens_ofs+3:sens_ofs+5]))
        
        return ts,T,H

    seen_eof = False

    records_pre = []
    records = []

    for i,ofs in enumerate(xrange(100,eof_marker,record_len)):
        rdata = data[ofs:ofs+record_len]

        if len(rdata) != record_len:
            print "natural (final) EOF encountered at offset 0x%x (i=%d)" % (ofs,i)
            break

        ts,T,H = parse_record(rdata, sensor_cnt)

        if not ts:
            if not buffer_overflowed or seen_eof:
                print "final EOF timestamp encountered at offset 0x%x (i=%d)" % (ofs,i)
                break

            print "interim EOF timestamp encountered at offset 0x%x (i=%d)" % (ofs,i)
            seen_eof = True
            continue

        if seen_eof:
            records_pre.append( (ts,T,H) )
        else:
            records.append( (ts,T,H) )


    print "-"*79

    line = "i|timestamp" + "".join("|T%d|H%d" % (n,n) for n in xrange(sensor_cnt+1))
    print line

    for i,(ts,T,H) in enumerate(records_pre+records):
        line = ("%d|%s" % ((i+1), ts)).replace(' ','T')

        for t,h in zip(T,H):
            if t is None:
                t = ""
            else:
                t = "%.1f" % (t/10.)

            if h is None:
                h = ""
            else:
                h = "%d" % h

            line += "|%s|%s" % (t,h)
        print line


if __name__ == '__main__':
    map(parse_file, sys.argv[1:])
