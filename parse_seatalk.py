import sys


def getByte(str):
    str = str[2:]
    if len(str) == 1:
	str = "0" + str;
    return ord(str.decode("hex"))


while 1:
    try:
        line = sys.stdin.readline().rstrip('\n')
    except KeyboardInterrupt:
        break

    if not line:
        break

    bytes = line.split(" ")
    datagram = getByte(bytes[0])

    if datagram == ord('\x20'):
	byte2 = getByte(bytes[2])
	byte3 = getByte(bytes[3])

	STW = (byte2*256+byte3)/100
	print "===> STW = " + str(STW) + "           " + line

    if datagram == ord('\x21'):
	byte2 = getByte(bytes[2])
	byte3 = getByte(bytes[3])

	x = (byte2*256+byte3)/100
	print "===> Trip mileage = " + str(x) + "           " + line

    if datagram == ord('\x22'):
	byte2 = getByte(bytes[2])
	byte3 = getByte(bytes[3])

	x = (byte2*256+byte3)/100
	print "===> Total mileage = " + str(x) + "           " + line

    if datagram == ord('\x23'):
	byte2 = getByte(bytes[2])
	byte3 = getByte(bytes[3])

	x = (byte2*256+byte3)/100
	print "===> Water temperature (C) = " + str(byte2) + "           " + line

    if datagram == ord('\x89'):
	U2 = getByte(bytes[1])
	VW = getByte(bytes[2])
	XY = getByte(bytes[3])
#	2Z = getByte(bytes[4])

	x = ((U2 // 16) & ord('\x03')) * 90 + (VW & ord('\x3f')) * 2 + ((U2 // 16 // 8) & ord('\x0c'))
	print "===> HDG = " + str(x) + "           " + line

    if datagram == ord('\x9c'):
	U2 = getByte(bytes[1])
	VW = getByte(bytes[2])
	XY = getByte(bytes[3])
#	2Z = getByte(bytes[4])

	x = ((U2 // 16) & ord('\x03')) * 90 + (VW & ord('\x3f')) * 2 + ((U2 // 16 // 8) & ord('\x0c'))
	print "===> HDG2 = " + str(x) + "           " + line

