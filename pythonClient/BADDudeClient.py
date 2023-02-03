import serial

BADDUDE_MAX_CHUNK_SIZE = 64

BADDUDE_STARTKEY = 0x89ABCDEF

BADDUDE_CMD_NOP = 0x00
BADDUDE_CMD_ACK = 0x01
BADDUDE_CMD_NACK = 0x02
BADDUDE_CMD_CPLT = 0x03
BADDUDE_CMD_NEXTCHUNK = 0x04
BADDUDE_CMD_PING = 0x05

BADDUDE_CMD_TPIENTER = 0x10
BADDUDE_CMD_TPIEXIT = 0x11

BADDUDE_CMD_FPRIME = 0x20
BADDUDE_CMD_FERASE = 0x21
BADDUDE_CMD_FWRITE = 0x22
BADDUDE_CMD_FWPROG = 0x23


def writeInt8(int8: int):
    ser.write(int8.to_bytes(1, byteorder='little'))


def writeInt16(int16: int):
    ser.write(int16.to_bytes(2, byteorder='little'))


def writeBytes(bytes: bytes):
    ser.write(bytes)


def readByte() -> int:
    return int.from_bytes(ser.read(1), byteorder='little')


def pollStartKey():
    remainingKey = BADDUDE_STARTKEY

    while True:
        receivedByte = readByte()
        if receivedByte == (remainingKey & 0xFF):
            remainingKey = remainingKey >> 8
            if remainingKey == 0:
                return
        else:
            remainingKey = BADDUDE_STARTKEY


def ping() -> bool:
    writeInt8(BADDUDE_CMD_PING)
    return readByte() == BADDUDE_CMD_ACK


def flashWrite(bytes: bytes):
    writeInt8(BADDUDE_CMD_FWRITE)
    writeInt16(len(bytes))

    for i in range(0, len(bytes), BADDUDE_MAX_CHUNK_SIZE):
        chunk = bytes[i:i+BADDUDE_MAX_CHUNK_SIZE]
        writeBytes(chunk)
        if readByte() != BADDUDE_CMD_NEXTCHUNK:
            print("Unexpected response from device")

    if (readByte() == BADDUDE_CMD_ACK):
        print("Transmitted program")

    while True:
        if (readByte() == BADDUDE_CMD_FWPROG):
            progress = readByte()
            print(
                "\rFlashing progress:",
                str(round((progress/255)*100)) + "%",
                end=""
            )
            if progress == 0xFF:
                print()
                return


ser = serial.Serial()
ser.port = "/dev/cu.usbmodem101"
ser.baudrate = 115200
ser.timeout = None
ser.open()
print("Opened serial port: " + ser.portstr +
      " (" + str(ser.baudrate) + " baud)")

ser.rts = True
ser.dtr = False

pollStartKey()
print("Start key received")

ping()
print("Ping successful")

writeInt8(BADDUDE_CMD_TPIENTER)
if (readByte() != BADDUDE_CMD_ACK):
    print("Failed to enter TPI mode")
    exit(1)
print("Entered TPI mode")

writeInt8(BADDUDE_CMD_FPRIME)
if (readByte() != BADDUDE_CMD_ACK):
    print("Failed to prime flash")
    exit(1)
print("Primed flash")

writeInt8(BADDUDE_CMD_FERASE)
if (readByte() != BADDUDE_CMD_ACK):
    print("Failed to erase flash")
    exit(1)
print("Erased flash")

programData = b'\x10\xc0\x17\xc0\x16\xc0\x15\xc0\x14\xc0\x13\xc0\x12\xc0\x11\xc0\x10\xc0\x0f\xc0\x0e\xc0\x0d\xc0\x0c\xc0\x0b\xc0\x0a\xc0\x09\xc0\x08\xc0\x11\x27\x1f\xbf\xcf\xeb\xd0\xe0\xde\xbf\xcd\xbf\x02\xd0\x41\xc0\xe6\xcf\xcf\x93\xdf\x93\x00\xd0\x00\xd0\x00\xd0\xcd\xb7\xde\xb7\x41\xe0\x50\xe0\xce\x5f\xdf\x4f\x58\x83\x4a\x93\xc1\x50\xd0\x40\x42\xe0\x50\xe0\xcc\x5f\xdf\x4f\x58\x83\x4a\x93\xc3\x50\xd0\x40\xcd\x5f\xdf\x4f\x49\x91\x58\x81\xc4\x50\xd0\x40\x60\xe2\xe4\x2f\xf5\x2f\x60\x83\xcf\x5f\xdf\x4f\x49\x91\x58\x81\xc2\x50\xd0\x40\x60\xe2\xe4\x2f\xf5\x2f\x60\x83\x40\xe0\x50\xe0\xca\x5f\xdf\x4f\x58\x83\x4a\x93\xc5\x50\xd0\x40\xcb\x5f\xdf\x4f\x89\x91\x98\x81\x0f\x91\x0f\x91\x0f\x91\x0f\x91\x0f\x91\x0f\x91\xdf\x91\xcf\x91\x08\x95\xf8\x94\xff\xcf'
flashWrite(programData)

writeInt8(BADDUDE_CMD_TPIEXIT)
# if (readByte() != BADDUDE_CMD_ACK):
#     print("Failed to exit TPI mode")
#     exit(1)
print("Exited TPI mode")

while True:
    print(ser.read(1).decode("utf-8"), end="")
